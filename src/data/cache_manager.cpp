#include "data/cache_manager.h"
#include "external/json.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

namespace trading
{
    CacheManager::CacheManager(size_t max_memory_mb, const std::string &cache_dir)
        : max_memory_bytes_(max_memory_mb * 1024 * 1024), current_memory_bytes_(0), cache_dir_(cache_dir), thread_pool_(nullptr)
    {
        // Create cache directory if it doesn't exist
        std::filesystem::create_directories(cache_dir_);

        // Load existing cache metadata
        load_cache_metadata();
    }

    CacheManager::CacheManager(size_t max_memory_mb, const std::string &cache_dir, std::shared_ptr<ThreadPool> thread_pool)
        : max_memory_bytes_(max_memory_mb * 1024 * 1024), current_memory_bytes_(0), cache_dir_(cache_dir), thread_pool_(thread_pool)
    {
        // Create cache directory if it doesn't exist
        std::filesystem::create_directories(cache_dir_);

        // Load existing cache metadata
        load_cache_metadata();
    }

    CacheManager::~CacheManager()
    {
        // Save cache metadata before destruction
        save_cache_metadata();
    }

    std::optional<MarketDataSeries> CacheManager::get(const std::string &key)
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        auto it = cache_.find(key);
        if (it == cache_.end())
        {
            return std::nullopt;
        }

        // Update access time for LRU
        it->second.last_accessed = std::chrono::steady_clock::now();

        // Move to front of LRU list
        lru_list_.remove(key);
        lru_list_.push_front(key);

        return it->second.data;
    }

    void CacheManager::put(const std::string &key, const MarketDataSeries &data)
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        // Calculate memory usage of this data
        size_t data_size = estimate_memory_usage(data);

        // Check if we need to evict items
        while (current_memory_bytes_ + data_size > max_memory_bytes_ && !cache_.empty())
        {
            evict_lru_item();
        }

        // If still too large, don't cache
        if (current_memory_bytes_ + data_size > max_memory_bytes_)
        {
            return;
        }

        // Add to cache
        CacheEntry entry;
        entry.data = data;
        entry.size_bytes = data_size;
        entry.last_accessed = std::chrono::steady_clock::now();
        entry.created = std::chrono::steady_clock::now();

        // Remove existing entry if it exists
        auto existing_it = cache_.find(key);
        if (existing_it != cache_.end())
        {
            current_memory_bytes_ -= existing_it->second.size_bytes;
            lru_list_.remove(key);
        }

        cache_[key] = entry;
        current_memory_bytes_ += data_size;
        lru_list_.push_front(key);

        // Persist to disk asynchronously if thread pool is available
        if (thread_pool_)
        {
            thread_pool_->submit([this, key, data]()
                                 { persist_to_disk(key, data); });
        }
        else
        {
            persist_to_disk(key, data);
        }
    }

    void CacheManager::remove(const std::string &key)
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        auto it = cache_.find(key);
        if (it != cache_.end())
        {
            current_memory_bytes_ -= it->second.size_bytes;
            lru_list_.remove(key);
            cache_.erase(it);

            // Remove from disk
            std::filesystem::remove(get_cache_file_path(key));
        }
    }

    void CacheManager::clear()
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        cache_.clear();
        lru_list_.clear();
        current_memory_bytes_ = 0;

        // Clear disk cache
        for (const auto &entry : std::filesystem::directory_iterator(cache_dir_))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".cache")
            {
                std::filesystem::remove(entry.path());
            }
        }
    }

    bool CacheManager::contains(const std::string &key) const
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        return cache_.find(key) != cache_.end();
    }

    size_t CacheManager::size() const
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        return cache_.size();
    }

    size_t CacheManager::memory_usage() const
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        return current_memory_bytes_;
    }

    double CacheManager::hit_rate() const
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        if (total_requests_ == 0)
        {
            return 0.0;
        }

        return static_cast<double>(cache_hits_) / total_requests_;
    }

    void CacheManager::cleanup_expired_entries(std::chrono::hours max_age)
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        auto now = std::chrono::steady_clock::now();
        std::vector<std::string> to_remove;

        for (const auto &[key, entry] : cache_)
        {
            if (now - entry.created > max_age)
            {
                to_remove.push_back(key);
            }
        }

        for (const auto &key : to_remove)
        {
            remove(key);
        }
    }

    void CacheManager::preload_from_disk()
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        for (const auto &entry : std::filesystem::directory_iterator(cache_dir_))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".cache")
            {
                std::string key = entry.path().stem().string();

                try
                {
                    auto data = load_from_disk(key);
                    if (data)
                    {
                        put(key, *data);
                    }
                }
                catch (const std::exception &e)
                {
                    // Log error and continue
                    std::cerr << "Failed to preload cache entry " << key << ": " << e.what() << std::endl;
                }
            }
        }
    }

    void CacheManager::evict_lru_item()
    {
        if (lru_list_.empty())
        {
            return;
        }

        std::string key_to_evict = lru_list_.back();
        lru_list_.pop_back();

        auto it = cache_.find(key_to_evict);
        if (it != cache_.end())
        {
            current_memory_bytes_ -= it->second.size_bytes;
            cache_.erase(it);
        }
    }

    size_t CacheManager::estimate_memory_usage(const MarketDataSeries &data) const
    {
        // Rough estimation: each data point is about 64 bytes (timestamp + 5 doubles + volume)
        size_t base_size = sizeof(MarketDataPoint) * data.size();

        // Add overhead for symbol string and vector storage
        size_t overhead = data.symbol().size() + sizeof(std::vector<MarketDataPoint>) + 100;

        return base_size + overhead;
    }

    std::string CacheManager::get_cache_file_path(const std::string &key) const
    {
        return (std::filesystem::path(cache_dir_) / (key + ".cache")).string();
    }

    void CacheManager::persist_to_disk(const std::string &key, const MarketDataSeries &data)
    {
        try
        {
            std::string filepath = get_cache_file_path(key);

            // Create JSON representation
            json j;
            j["symbol"] = data.symbol();
            j["data"] = json::array();

            for (const auto &point : data.data())
            {
                json point_json;
                point_json["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                                              point.timestamp.time_since_epoch())
                                              .count();
                point_json["open"] = point.open;
                point_json["high"] = point.high;
                point_json["low"] = point.low;
                point_json["close"] = point.close;
                point_json["volume"] = point.volume;

                j["data"].push_back(point_json);
            }

            // Write to file
            std::ofstream file(filepath);
            if (file.is_open())
            {
                file << j.dump(2);
                file.close();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to persist cache entry " << key << ": " << e.what() << std::endl;
        }
    }

    std::optional<MarketDataSeries> CacheManager::load_from_disk(const std::string &key)
    {
        try
        {
            std::string filepath = get_cache_file_path(key);
            std::ifstream file(filepath);

            if (!file.is_open())
            {
                return std::nullopt;
            }

            json j;
            file >> j;

            MarketDataSeries series(j["symbol"].get<std::string>());

            for (const auto &point_json : j["data"])
            {
                MarketDataPoint point(
                    std::chrono::system_clock::from_time_t(point_json["timestamp"].get<int64_t>()),
                    point_json["open"].get<double>(),
                    point_json["high"].get<double>(),
                    point_json["low"].get<double>(),
                    point_json["close"].get<double>(),
                    point_json["volume"].get<int64_t>());

                series.add_point(std::move(point));
            }

            return series;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to load cache entry " << key << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    void CacheManager::load_cache_metadata()
    {
        try
        {
            std::string metadata_file = (std::filesystem::path(cache_dir_) / "metadata.json").string();
            std::ifstream file(metadata_file);

            if (file.is_open())
            {
                json j;
                file >> j;

                total_requests_ = j.value("total_requests", 0);
                cache_hits_ = j.value("cache_hits", 0);
            }
        }
        catch (const std::exception &e)
        {
            // Metadata loading failed, use defaults
            total_requests_ = 0;
            cache_hits_ = 0;
        }
    }

    void CacheManager::save_cache_metadata()
    {
        try
        {
            std::string metadata_file = (std::filesystem::path(cache_dir_) / "metadata.json").string();
            std::ofstream file(metadata_file);

            if (file.is_open())
            {
                json j;
                j["total_requests"] = total_requests_;
                j["cache_hits"] = cache_hits_;

                file << j.dump(2);
                file.close();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to save cache metadata: " << e.what() << std::endl;
        }
    }
}
#pragma once

#include "data/market_data.h"
#include "core/thread_pool.h"
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <chrono>
#include <memory>
#include <optional>

namespace trading
{

    /**
     * @brief Cache entry for market data
     */
    struct CacheEntry
    {
        MarketDataSeries data;
        size_t size_bytes;
        std::chrono::steady_clock::time_point last_accessed;
        std::chrono::steady_clock::time_point created;

        CacheEntry() = default;
    };

    /**
     * @brief Market data cache manager with LRU eviction and disk persistence
     */
    class CacheManager
    {
    public:
        /**
         * @brief Constructor
         * @param max_memory_mb Maximum memory usage in MB
         * @param cache_dir Directory for disk cache
         */
        CacheManager(size_t max_memory_mb, const std::string &cache_dir);

        /**
         * @brief Constructor with thread pool
         * @param max_memory_mb Maximum memory usage in MB
         * @param cache_dir Directory for disk cache
         * @param thread_pool Thread pool for async operations
         */
        CacheManager(size_t max_memory_mb, const std::string &cache_dir, std::shared_ptr<ThreadPool> thread_pool);

        /**
         * @brief Destructor
         */
        ~CacheManager();

        // Prevent copying
        CacheManager(const CacheManager &) = delete;
        CacheManager &operator=(const CacheManager &) = delete;

        // Allow moving
        CacheManager(CacheManager &&other) noexcept;
        CacheManager &operator=(CacheManager &&other) noexcept;

        /**
         * @brief Get cached data
         * @param key Cache key
         * @return Cached data if available
         */
        std::optional<MarketDataSeries> get(const std::string &key);

        /**
         * @brief Store data in cache
         * @param key Cache key
         * @param data Market data to cache
         */
        void put(const std::string &key, const MarketDataSeries &data);

        /**
         * @brief Remove cache entry
         * @param key Cache key
         */
        void remove(const std::string &key);

        /**
         * @brief Clear all cache entries
         */
        void clear();

        /**
         * @brief Check if data is cached
         * @param key Cache key
         * @return true if cached
         */
        bool contains(const std::string &key) const;

        /**
         * @brief Get cache size
         * @return Number of cache entries
         */
        size_t size() const;

        /**
         * @brief Get memory usage
         * @return Memory usage in bytes
         */
        size_t memory_usage() const;

        /**
         * @brief Get cache hit rate
         * @return Hit rate as percentage
         */
        double hit_rate() const;

        /**
         * @brief Clean up expired entries
         * @param max_age Maximum age for entries
         */
        void cleanup_expired_entries(std::chrono::hours max_age);

        /**
         * @brief Preload cache from disk
         */
        void preload_from_disk();

    private:
        // Helper methods
        void evict_lru_item();
        size_t estimate_memory_usage(const MarketDataSeries &data) const;
        std::string get_cache_file_path(const std::string &key) const;
        void persist_to_disk(const std::string &key, const MarketDataSeries &data);
        std::optional<MarketDataSeries> load_from_disk(const std::string &key);
        void load_cache_metadata();
        void save_cache_metadata();

        // Member variables
        mutable std::mutex cache_mutex_;
        std::unordered_map<std::string, CacheEntry> cache_;
        std::list<std::string> lru_list_; // Most recently used first

        size_t max_memory_bytes_;
        size_t current_memory_bytes_;
        std::string cache_dir_;
        std::shared_ptr<ThreadPool> thread_pool_;

        // Statistics
        size_t total_requests_{0};
        size_t cache_hits_{0};
    };

} // namespace trading
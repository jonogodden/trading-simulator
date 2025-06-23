#include "data/yahoo_finance.h"
#include "external/json.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>

using json = nlohmann::json;

namespace trading
{
    YahooFinanceClient::YahooFinanceClient(std::shared_ptr<ThreadPool> thread_pool)
        : thread_pool_(thread_pool), curl_handle_(nullptr)
    {
        // Initialize CURL
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle_ = curl_easy_init();

        if (!curl_handle_)
        {
            throw std::runtime_error("Failed to initialize CURL");
        }

        // Set default CURL options
        curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT, timeout_seconds_);
        curl_easy_setopt(curl_handle_, CURLOPT_USERAGENT, user_agent_.c_str());
        curl_easy_setopt(curl_handle_, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, write_callback);
    }

    YahooFinanceClient::~YahooFinanceClient()
    {
        if (curl_handle_)
        {
            curl_easy_cleanup(curl_handle_);
        }
        curl_global_cleanup();
    }

    YahooFinanceClient::YahooFinanceClient(YahooFinanceClient &&other) noexcept
        : thread_pool_(std::move(other.thread_pool_)), curl_handle_(other.curl_handle_), timeout_seconds_(other.timeout_seconds_), max_retries_(other.max_retries_), user_agent_(std::move(other.user_agent_))
    {
        other.curl_handle_ = nullptr;
    }

    YahooFinanceClient &YahooFinanceClient::operator=(YahooFinanceClient &&other) noexcept
    {
        if (this != &other)
        {
            if (curl_handle_)
            {
                curl_easy_cleanup(curl_handle_);
            }

            thread_pool_ = std::move(other.thread_pool_);
            curl_handle_ = other.curl_handle_;
            timeout_seconds_ = other.timeout_seconds_;
            max_retries_ = other.max_retries_;
            user_agent_ = std::move(other.user_agent_);

            other.curl_handle_ = nullptr;
        }
        return *this;
    }

    std::future<MarketDataSeries> YahooFinanceClient::fetch_historical_data(const MarketDataRequest &request)
    {
        return thread_pool_->submit([this, request]()
                                    { return fetch_historical_data_sync(request); });
    }

    MarketDataSeries YahooFinanceClient::fetch_historical_data_sync(const MarketDataRequest &request)
    {
        std::string url = build_url(request);
        std::string response;

        // Set up CURL for this request
        curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &response);

        // Perform the request with retries
        CURLcode result;
        int retry_count = 0;

        do
        {
            response.clear();
            result = curl_easy_perform(curl_handle_);

            if (result != CURLE_OK)
            {
                retry_count++;
                if (retry_count < max_retries_)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000 * retry_count));
                    continue;
                }
                handle_curl_error(result, "fetch_historical_data");
            }
        } while (result != CURLE_OK && retry_count < max_retries_);

        if (result != CURLE_OK)
        {
            throw std::runtime_error("Failed to fetch data after " + std::to_string(max_retries_) + " retries");
        }

        return parse_json_response(response, request.symbol);
    }

    double YahooFinanceClient::get_current_price(const std::string &symbol)
    {
        // For now, return a placeholder value
        // In a real implementation, you would fetch the current price
        return 100.0;
    }

    bool YahooFinanceClient::validate_symbol(const std::string &symbol)
    {
        // For now, just check if the symbol is not empty
        return !symbol.empty();
    }

    size_t YahooFinanceClient::write_callback(void *contents, size_t size, size_t nmemb, std::string *userp)
    {
        userp->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    std::string YahooFinanceClient::build_url(const MarketDataRequest &request) const
    {
        // Convert timestamps to Unix timestamps
        auto start_ts = std::chrono::duration_cast<std::chrono::seconds>(
                            request.start_date.time_since_epoch())
                            .count();
        auto end_ts = std::chrono::duration_cast<std::chrono::seconds>(
                          request.end_date.time_since_epoch())
                          .count();

        // Build Yahoo Finance API URL
        std::ostringstream url;
        url << "https://query1.finance.yahoo.com/v8/finance/chart/"
            << request.symbol
            << "?period1=" << start_ts
            << "&period2=" << end_ts
            << "&interval=" << request.interval
            << "&includePrePost=false"
            << "&events=div%2Csplit";

        return url.str();
    }

    MarketDataSeries YahooFinanceClient::parse_json_response(const std::string &json_data, const std::string &symbol) const
    {
        try
        {
            json j = json::parse(json_data);

            MarketDataSeries series(symbol);

            // Check if the response contains data
            if (j["chart"]["error"].is_object())
            {
                throw std::runtime_error("Yahoo Finance API error: " + j["chart"]["error"]["description"].get<std::string>());
            }

            auto result = j["chart"]["result"][0];
            auto timestamp = result["timestamp"];
            auto indicators = result["indicators"]["quote"][0];

            // Get OHLCV data
            auto open = indicators["open"];
            auto high = indicators["high"];
            auto low = indicators["low"];
            auto close = indicators["close"];
            auto volume = indicators["volume"];

            // Process each data point
            for (size_t i = 0; i < timestamp.size(); ++i)
            {
                // Skip if any required data is missing
                if (open[i].is_null() || high[i].is_null() || low[i].is_null() || close[i].is_null())
                {
                    continue;
                }

                MarketDataPoint point(
                    parse_timestamp(timestamp[i].get<int64_t>()),
                    open[i].get<double>(),
                    high[i].get<double>(),
                    low[i].get<double>(),
                    close[i].get<double>(),
                    volume[i].is_null() ? 0 : volume[i].get<int64_t>());

                series.add_point(std::move(point));
            }

            return series;
        }
        catch (const json::exception &e)
        {
            throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
        }
    }

    std::chrono::system_clock::time_point YahooFinanceClient::parse_timestamp(int64_t timestamp) const
    {
        return std::chrono::system_clock::from_time_t(timestamp);
    }

    void YahooFinanceClient::handle_curl_error(CURLcode result, const std::string &operation) const
    {
        std::string error_msg = "CURL error in " + operation + ": " + get_error_message(result);
        throw std::runtime_error(error_msg);
    }

    std::string YahooFinanceClient::get_error_message(CURLcode result) const
    {
        return curl_easy_strerror(result);
    }
}
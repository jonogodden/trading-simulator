#pragma once

#include "data/market_data.h"
#include "core/thread_pool.h"
#include <string>
#include <future>
#include <memory>
#include <curl/curl.h>

namespace trading
{

    /**
     * @brief Yahoo Finance API client for fetching market data
     *
     * This class provides an interface to the Yahoo Finance API for
     * fetching historical market data. It handles HTTP requests,
     * JSON parsing, and data validation.
     *
     * Key features:
     * - Asynchronous data fetching using thread pool
     * - Automatic retry on failures
     * - Rate limiting to respect API limits
     * - Data validation and error handling
     */
    class YahooFinanceClient
    {
    public:
        /**
         * @brief Constructor
         * @param thread_pool Thread pool for async operations
         *
         * Creates a Yahoo Finance client with the specified thread pool
         * for handling asynchronous data requests.
         */
        explicit YahooFinanceClient(std::shared_ptr<ThreadPool> thread_pool);

        /**
         * @brief Destructor
         *
         * Cleans up CURL resources.
         */
        ~YahooFinanceClient();

        // Prevent copying
        YahooFinanceClient(const YahooFinanceClient &) = delete;
        YahooFinanceClient &operator=(const YahooFinanceClient &) = delete;

        // Allow moving
        YahooFinanceClient(YahooFinanceClient &&) noexcept;
        YahooFinanceClient &operator=(YahooFinanceClient &&) noexcept;

        /**
         * @brief Fetch historical market data
         * @param request Market data request parameters
         * @return Future containing the market data series
         *
         * Asynchronously fetches historical market data for the specified
         * symbol and time range. Returns a future that will contain the
         * result when the operation completes.
         */
        std::future<MarketDataSeries> fetch_historical_data(const MarketDataRequest &request);

        /**
         * @brief Fetch historical market data (synchronous)
         * @param request Market data request parameters
         * @return Market data series
         *
         * Synchronously fetches historical market data. This method blocks
         * until the data is retrieved or an error occurs.
         */
        MarketDataSeries fetch_historical_data_sync(const MarketDataRequest &request);

        /**
         * @brief Get current stock price
         * @param symbol Stock symbol
         * @return Current price
         *
         * Fetches the current stock price for the specified symbol.
         */
        double get_current_price(const std::string &symbol);

        /**
         * @brief Validate symbol
         * @param symbol Stock symbol to validate
         * @return true if symbol is valid
         *
         * Checks if the given symbol exists and is valid.
         */
        bool validate_symbol(const std::string &symbol);

        /**
         * @brief Set request timeout
         * @param timeout_seconds Timeout in seconds
         *
         * Sets the timeout for HTTP requests.
         */
        void set_timeout(int timeout_seconds) { timeout_seconds_ = timeout_seconds; }

        /**
         * @brief Set retry attempts
         * @param max_retries Maximum number of retry attempts
         *
         * Sets the maximum number of retry attempts for failed requests.
         */
        void set_max_retries(int max_retries) { max_retries_ = max_retries; }

    private:
        // HTTP response callback
        static size_t write_callback(void *contents, size_t size, size_t nmemb, std::string *userp);

        // Helper methods
        std::string build_url(const MarketDataRequest &request) const;
        MarketDataSeries parse_json_response(const std::string &json_data, const std::string &symbol) const;
        std::chrono::system_clock::time_point parse_timestamp(int64_t timestamp) const;

        // Error handling
        void handle_curl_error(CURLcode result, const std::string &operation) const;
        std::string get_error_message(CURLcode result) const;

        // Member variables
        std::shared_ptr<ThreadPool> thread_pool_;        // Thread pool for async operations
        CURL *curl_handle_;                              // CURL handle for HTTP requests
        int timeout_seconds_{30};                        // Request timeout
        int max_retries_{3};                             // Maximum retry attempts
        std::string user_agent_{"TradingSimulator/1.0"}; // User agent string
    };

} // namespace trading
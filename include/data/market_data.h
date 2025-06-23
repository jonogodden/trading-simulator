#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>

namespace trading
{

    /**
     * @brief Market data point containing OHLCV information
     *
     * This structure represents a single data point in a time series,
     * containing the standard Open, High, Low, Close, Volume data
     * used in financial analysis.
     */
    struct MarketDataPoint
    {
        std::chrono::system_clock::time_point timestamp; // When this data point occurred
        double open;                                     // Opening price
        double high;                                     // Highest price during period
        double low;                                      // Lowest price during period
        double close;                                    // Closing price
        int64_t volume;                                  // Trading volume

        MarketDataPoint() = default;

        MarketDataPoint(std::chrono::system_clock::time_point ts,
                        double o, double h, double l, double c, int64_t v)
            : timestamp(ts), open(o), high(h), low(l), close(c), volume(v) {}
    };

    /**
     * @brief Time series of market data
     *
     * This class holds a collection of market data points for a specific
     * symbol over a time period. It provides methods for accessing and
     * manipulating the data efficiently.
     */
    class MarketDataSeries
    {
    public:
        MarketDataSeries() = default;
        explicit MarketDataSeries(const std::string &symbol) : symbol_(symbol) {}

        // Getters
        const std::string &symbol() const { return symbol_; }
        const std::vector<MarketDataPoint> &data() const { return data_; }
        size_t size() const { return data_.size(); }
        bool empty() const { return data_.empty(); }

        // Data access
        const MarketDataPoint &operator[](size_t index) const { return data_[index]; }
        const MarketDataPoint &front() const { return data_.front(); }
        const MarketDataPoint &back() const { return data_.back(); }

        // Data modification
        void add_point(const MarketDataPoint &point) { data_.push_back(point); }
        void add_point(MarketDataPoint &&point) { data_.emplace_back(std::move(point)); }
        void clear() { data_.clear(); }
        void reserve(size_t capacity) { data_.reserve(capacity); }

        // Time range queries
        std::vector<MarketDataPoint> get_range(
            std::chrono::system_clock::time_point start,
            std::chrono::system_clock::time_point end) const;

        // Statistical methods
        double get_average_price() const;
        double get_volatility() const;
        double get_max_price() const;
        double get_min_price() const;

        // Validation
        bool is_valid() const;

    private:
        std::string symbol_;                // Stock symbol (e.g., "AAPL")
        std::vector<MarketDataPoint> data_; // Time series data
    };

    /**
     * @brief Market data request parameters
     *
     * This structure contains all the parameters needed to request
     * market data from a data provider.
     */
    struct MarketDataRequest
    {
        std::string symbol;                               // Stock symbol
        std::chrono::system_clock::time_point start_date; // Start date
        std::chrono::system_clock::time_point end_date;   // End date
        std::string interval = "1d";                      // Data interval (1d, 1h, 5m, etc.)

        MarketDataRequest() = default;

        MarketDataRequest(const std::string &sym,
                          std::chrono::system_clock::time_point start,
                          std::chrono::system_clock::time_point end,
                          const std::string &intrvl = "1d")
            : symbol(sym), start_date(start), end_date(end), interval(intrvl) {}
    };

} // namespace trading
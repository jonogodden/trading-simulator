#pragma once

#include "data/market_data.h"
#include <vector>
#include <string>

namespace trading
{

    /**
     * @brief Technical indicators for market analysis
     *
     * This structure contains commonly used technical indicators
     * calculated from market data.
     */
    struct TechnicalIndicators
    {
        std::vector<double> sma_20;          // 20-period Simple Moving Average
        std::vector<double> sma_50;          // 50-period Simple Moving Average
        std::vector<double> ema_12;          // 12-period Exponential Moving Average
        std::vector<double> ema_26;          // 26-period Exponential Moving Average
        std::vector<double> rsi;             // Relative Strength Index
        std::vector<double> macd;            // MACD line
        std::vector<double> macd_signal;     // MACD signal line
        std::vector<double> bollinger_upper; // Bollinger Bands upper
        std::vector<double> bollinger_lower; // Bollinger Bands lower
        std::vector<double> volume_sma;      // Volume Simple Moving Average
    };

    /**
     * @brief Data processing and analysis engine
     *
     * This class provides methods for cleaning, validating, and analyzing
     * market data. It includes technical indicator calculations and
     * data quality checks.
     *
     * Key features:
     * - Data cleaning and validation
     * - Technical indicator calculations
     * - Outlier detection
     * - Data normalization
     * - Statistical analysis
     */
    class DataProcessor
    {
    public:
        DataProcessor() = default;

        /**
         * @brief Clean and validate market data
         * @param series Raw market data series
         * @return Cleaned and validated data series
         *
         * Removes outliers, fills missing data, and validates the
         * integrity of the market data.
         */
        MarketDataSeries clean_data(const MarketDataSeries &series);

        /**
         * @brief Calculate technical indicators
         * @param series Market data series
         * @return Technical indicators
         *
         * Calculates various technical indicators from the market data.
         */
        TechnicalIndicators calculate_indicators(const MarketDataSeries &series);

        /**
         * @brief Calculate Simple Moving Average
         * @param prices Price data
         * @param period Period for calculation
         * @return Moving average values
         */
        std::vector<double> calculate_sma(const std::vector<double> &prices, int period) const;

        /**
         * @brief Calculate Exponential Moving Average
         * @param prices Price data
         * @param period Period for calculation
         * @return Exponential moving average values
         */
        std::vector<double> calculate_ema(const std::vector<double> &prices, int period) const;

        /**
         * @brief Calculate Relative Strength Index
         * @param prices Price data
         * @param period Period for calculation (default 14)
         * @return RSI values
         */
        std::vector<double> calculate_rsi(const std::vector<double> &prices, int period = 14) const;

        /**
         * @brief Calculate MACD
         * @param prices Price data
         * @param fast_period Fast EMA period (default 12)
         * @param slow_period Slow EMA period (default 26)
         * @param signal_period Signal line period (default 9)
         * @return MACD line and signal line
         */
        std::pair<std::vector<double>, std::vector<double>> calculate_macd(
            const std::vector<double> &prices,
            int fast_period = 12,
            int slow_period = 26,
            int signal_period = 9) const;

        /**
         * @brief Calculate Bollinger Bands
         * @param prices Price data
         * @param period Period for calculation (default 20)
         * @param std_dev Standard deviation multiplier (default 2.0)
         * @return Upper and lower bands
         */
        std::pair<std::vector<double>, std::vector<double>> calculate_bollinger_bands(
            const std::vector<double> &prices,
            int period = 20,
            double std_dev = 2.0) const;

        /**
         * @brief Detect outliers in price data
         * @param prices Price data
         * @param threshold Standard deviation threshold (default 3.0)
         * @return Indices of outlier points
         */
        std::vector<size_t> detect_outliers(const std::vector<double> &prices, double threshold = 3.0) const;

        /**
         * @brief Fill missing data points
         * @param series Market data series with gaps
         * @return Series with missing data filled
         *
         * Interpolates missing data points using linear interpolation.
         */
        MarketDataSeries fill_missing_data(const MarketDataSeries &series) const;

        /**
         * @brief Normalize price data
         * @param prices Price data
         * @return Normalized prices (0-1 range)
         */
        std::vector<double> normalize_prices(const std::vector<double> &prices) const;

        /**
         * @brief Calculate price returns
         * @param prices Price data
         * @return Price returns (percentage change)
         */
        std::vector<double> calculate_returns(const std::vector<double> &prices) const;

        /**
         * @brief Calculate volatility
         * @param returns Price returns
         * @param window Rolling window size (default 20)
         * @return Rolling volatility
         */
        std::vector<double> calculate_volatility(const std::vector<double> &returns, int window = 20) const;

    private:
        // Helper methods
        double calculate_std_dev(const std::vector<double> &values, size_t start, size_t end) const;
        std::vector<double> calculate_gains_losses(const std::vector<double> &prices) const;
        bool is_valid_price(double price) const;
        bool is_valid_volume(int64_t volume) const;
    };

} // namespace trading
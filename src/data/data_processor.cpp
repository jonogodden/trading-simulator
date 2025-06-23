#include "data/data_processor.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>

namespace trading
{
    MarketDataSeries DataProcessor::clean_data(const MarketDataSeries &series)
    {
        MarketDataSeries cleaned_series(series.symbol());

        if (series.empty())
        {
            return cleaned_series;
        }

        // Reserve space for efficiency
        cleaned_series.reserve(series.size());

        // Extract prices for outlier detection
        std::vector<double> prices;
        prices.reserve(series.size());
        for (const auto &point : series.data())
        {
            prices.push_back(point.close);
        }

        // Detect outliers
        auto outlier_indices = detect_outliers(prices, 3.0);

        // Add non-outlier points to cleaned series
        for (size_t i = 0; i < series.size(); ++i)
        {
            if (std::find(outlier_indices.begin(), outlier_indices.end(), i) == outlier_indices.end())
            {
                cleaned_series.add_point(series[i]);
            }
        }

        return cleaned_series;
    }

    TechnicalIndicators DataProcessor::calculate_indicators(const MarketDataSeries &series)
    {
        TechnicalIndicators indicators;

        if (series.empty())
        {
            return indicators;
        }

        // Extract close prices
        std::vector<double> prices;
        prices.reserve(series.size());
        for (const auto &point : series.data())
        {
            prices.push_back(point.close);
        }

        // Calculate indicators
        indicators.sma_20 = calculate_sma(prices, 20);
        indicators.sma_50 = calculate_sma(prices, 50);
        indicators.ema_12 = calculate_ema(prices, 12);
        indicators.ema_26 = calculate_ema(prices, 26);
        indicators.rsi = calculate_rsi(prices, 14);

        auto macd_result = calculate_macd(prices, 12, 26, 9);
        indicators.macd = macd_result.first;
        indicators.macd_signal = macd_result.second;

        auto bollinger_result = calculate_bollinger_bands(prices, 20, 2.0);
        indicators.bollinger_upper = bollinger_result.first;
        indicators.bollinger_lower = bollinger_result.second;

        // Calculate volume SMA
        std::vector<double> volumes;
        volumes.reserve(series.size());
        for (const auto &point : series.data())
        {
            volumes.push_back(static_cast<double>(point.volume));
        }
        indicators.volume_sma = calculate_sma(volumes, 20);

        return indicators;
    }

    std::vector<double> DataProcessor::calculate_sma(const std::vector<double> &prices, int period) const
    {
        std::vector<double> sma;
        sma.reserve(prices.size());

        for (size_t i = 0; i < prices.size(); ++i)
        {
            if (i < static_cast<size_t>(period - 1))
            {
                sma.push_back(std::numeric_limits<double>::quiet_NaN());
            }
            else
            {
                double sum = 0.0;
                for (int j = 0; j < period; ++j)
                {
                    sum += prices[i - j];
                }
                sma.push_back(sum / period);
            }
        }

        return sma;
    }

    std::vector<double> DataProcessor::calculate_ema(const std::vector<double> &prices, int period) const
    {
        std::vector<double> ema;
        ema.reserve(prices.size());

        double multiplier = 2.0 / (period + 1);

        for (size_t i = 0; i < prices.size(); ++i)
        {
            if (i == 0)
            {
                ema.push_back(prices[0]);
            }
            else
            {
                double new_ema = (prices[i] * multiplier) + (ema[i - 1] * (1 - multiplier));
                ema.push_back(new_ema);
            }
        }

        return ema;
    }

    std::vector<double> DataProcessor::calculate_rsi(const std::vector<double> &prices, int period) const
    {
        std::vector<double> rsi;
        rsi.reserve(prices.size());

        if (prices.size() < 2)
        {
            return rsi;
        }

        // Calculate price changes
        std::vector<double> gains, losses;
        gains.reserve(prices.size());
        losses.reserve(prices.size());

        for (size_t i = 1; i < prices.size(); ++i)
        {
            double change = prices[i] - prices[i - 1];
            gains.push_back(change > 0 ? change : 0.0);
            losses.push_back(change < 0 ? -change : 0.0);
        }

        // Calculate RSI
        for (size_t i = 0; i < prices.size(); ++i)
        {
            if (i < static_cast<size_t>(period))
            {
                rsi.push_back(std::numeric_limits<double>::quiet_NaN());
            }
            else
            {
                double avg_gain = 0.0, avg_loss = 0.0;

                for (int j = 0; j < period; ++j)
                {
                    avg_gain += gains[i - period + j];
                    avg_loss += losses[i - period + j];
                }

                avg_gain /= period;
                avg_loss /= period;

                if (avg_loss == 0.0)
                {
                    rsi.push_back(100.0);
                }
                else
                {
                    double rs = avg_gain / avg_loss;
                    rsi.push_back(100.0 - (100.0 / (1.0 + rs)));
                }
            }
        }

        return rsi;
    }

    std::pair<std::vector<double>, std::vector<double>> DataProcessor::calculate_macd(
        const std::vector<double> &prices, int fast_period, int slow_period, int signal_period) const
    {

        auto fast_ema = calculate_ema(prices, fast_period);
        auto slow_ema = calculate_ema(prices, slow_period);

        std::vector<double> macd_line;
        macd_line.reserve(prices.size());

        for (size_t i = 0; i < prices.size(); ++i)
        {
            if (std::isnan(fast_ema[i]) || std::isnan(slow_ema[i]))
            {
                macd_line.push_back(std::numeric_limits<double>::quiet_NaN());
            }
            else
            {
                macd_line.push_back(fast_ema[i] - slow_ema[i]);
            }
        }

        auto signal_line = calculate_ema(macd_line, signal_period);

        return {macd_line, signal_line};
    }

    std::pair<std::vector<double>, std::vector<double>> DataProcessor::calculate_bollinger_bands(
        const std::vector<double> &prices, int period, double std_dev_multiplier) const
    {

        std::vector<double> upper_band, lower_band;
        upper_band.reserve(prices.size());
        lower_band.reserve(prices.size());

        for (size_t i = 0; i < prices.size(); ++i)
        {
            if (i < static_cast<size_t>(period - 1))
            {
                upper_band.push_back(std::numeric_limits<double>::quiet_NaN());
                lower_band.push_back(std::numeric_limits<double>::quiet_NaN());
            }
            else
            {
                double sma = 0.0;
                for (int j = 0; j < period; ++j)
                {
                    sma += prices[i - j];
                }
                sma /= period;

                double variance = 0.0;
                for (int j = 0; j < period; ++j)
                {
                    double diff = prices[i - j] - sma;
                    variance += diff * diff;
                }
                variance /= period;

                double std_dev = std::sqrt(variance);

                upper_band.push_back(sma + (std_dev_multiplier * std_dev));
                lower_band.push_back(sma - (std_dev_multiplier * std_dev));
            }
        }

        return {upper_band, lower_band};
    }

    std::vector<size_t> DataProcessor::detect_outliers(const std::vector<double> &prices, double threshold) const
    {
        std::vector<size_t> outliers;

        if (prices.size() < 2)
        {
            return outliers;
        }

        // Calculate mean and standard deviation
        double mean = std::accumulate(prices.begin(), prices.end(), 0.0) / prices.size();

        double variance = 0.0;
        for (double price : prices)
        {
            double diff = price - mean;
            variance += diff * diff;
        }
        variance /= prices.size();

        double std_dev = std::sqrt(variance);

        // Find outliers
        for (size_t i = 0; i < prices.size(); ++i)
        {
            double z_score = std::abs(prices[i] - mean) / std_dev;
            if (z_score > threshold)
            {
                outliers.push_back(i);
            }
        }

        return outliers;
    }

    MarketDataSeries DataProcessor::fill_missing_data(const MarketDataSeries &series) const
    {
        MarketDataSeries filled_series(series.symbol());

        if (series.empty())
        {
            return filled_series;
        }

        const auto &data = series.data();
        filled_series.add_point(data[0]); // First point is always valid

        for (size_t i = 1; i < data.size(); ++i)
        {
            const auto &current = data[i];
            const auto &previous = data[i - 1];

            // Check if current point has missing data
            if (!is_valid_price(current.open) || !is_valid_price(current.high) ||
                !is_valid_price(current.low) || !is_valid_price(current.close))
            {

                // Use previous point's close price for missing values
                MarketDataPoint filled_point(
                    current.timestamp,
                    is_valid_price(current.open) ? current.open : previous.close,
                    is_valid_price(current.high) ? current.high : previous.close,
                    is_valid_price(current.low) ? current.low : previous.close,
                    is_valid_price(current.close) ? current.close : previous.close,
                    current.volume);
                filled_series.add_point(std::move(filled_point));
            }
            else
            {
                filled_series.add_point(current);
            }
        }

        return filled_series;
    }

    std::vector<double> DataProcessor::normalize_prices(const std::vector<double> &prices) const
    {
        if (prices.empty())
        {
            return {};
        }

        double min_price = *std::min_element(prices.begin(), prices.end());
        double max_price = *std::max_element(prices.begin(), prices.end());

        if (max_price == min_price)
        {
            return std::vector<double>(prices.size(), 0.5);
        }

        std::vector<double> normalized;
        normalized.reserve(prices.size());

        for (double price : prices)
        {
            normalized.push_back((price - min_price) / (max_price - min_price));
        }

        return normalized;
    }

    std::vector<double> DataProcessor::calculate_returns(const std::vector<double> &prices) const
    {
        std::vector<double> returns;
        returns.reserve(prices.size());

        if (prices.size() < 2)
        {
            return returns;
        }

        returns.push_back(0.0); // First return is 0

        for (size_t i = 1; i < prices.size(); ++i)
        {
            if (prices[i - 1] != 0.0)
            {
                double ret = (prices[i] - prices[i - 1]) / prices[i - 1];
                returns.push_back(ret);
            }
            else
            {
                returns.push_back(0.0);
            }
        }

        return returns;
    }

    std::vector<double> DataProcessor::calculate_volatility(const std::vector<double> &returns, int window) const
    {
        std::vector<double> volatility;
        volatility.reserve(returns.size());

        for (size_t i = 0; i < returns.size(); ++i)
        {
            if (i < static_cast<size_t>(window - 1))
            {
                volatility.push_back(std::numeric_limits<double>::quiet_NaN());
            }
            else
            {
                double std_dev = calculate_std_dev(returns, i - window + 1, i + 1);
                volatility.push_back(std_dev * std::sqrt(252.0)); // Annualized volatility
            }
        }

        return volatility;
    }

    double DataProcessor::calculate_std_dev(const std::vector<double> &values, size_t start, size_t end) const
    {
        if (start >= end || start >= values.size())
        {
            return 0.0;
        }

        end = std::min(end, values.size());

        double mean = 0.0;
        for (size_t i = start; i < end; ++i)
        {
            mean += values[i];
        }
        mean /= (end - start);

        double variance = 0.0;
        for (size_t i = start; i < end; ++i)
        {
            double diff = values[i] - mean;
            variance += diff * diff;
        }
        variance /= (end - start);

        return std::sqrt(variance);
    }

    std::vector<double> DataProcessor::calculate_gains_losses(const std::vector<double> &prices) const
    {
        std::vector<double> gains_losses;
        gains_losses.reserve(prices.size());

        if (prices.size() < 2)
        {
            return gains_losses;
        }

        gains_losses.push_back(0.0); // First value is 0

        for (size_t i = 1; i < prices.size(); ++i)
        {
            gains_losses.push_back(prices[i] - prices[i - 1]);
        }

        return gains_losses;
    }

    bool DataProcessor::is_valid_price(double price) const
    {
        return !std::isnan(price) && !std::isinf(price) && price > 0.0;
    }

    bool DataProcessor::is_valid_volume(int64_t volume) const
    {
        return volume >= 0;
    }
} // namespace trading
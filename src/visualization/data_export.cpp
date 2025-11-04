#include "visualization/data_export.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <cctype>

namespace trading
{
    namespace visualization
    {

        // ExportFactory implementation
        std::unique_ptr<DataExporter> ExportFactory::create_exporter(ExportFormat format)
        {
            switch (format)
            {
            case ExportFormat::CSV:
                return std::make_unique<CSVExporter>();
            case ExportFormat::JSON:
                return std::make_unique<JSONExporter>();
            case ExportFormat::XML:
                return std::make_unique<XMLExporter>();
            case ExportFormat::EXCEL:
                return std::make_unique<ExcelExporter>();
            default:
                return nullptr;
            }
        }

        std::string ExportFactory::get_file_extension(ExportFormat format)
        {
            switch (format)
            {
            case ExportFormat::CSV:
                return ".csv";
            case ExportFormat::JSON:
                return ".json";
            case ExportFormat::XML:
                return ".xml";
            case ExportFormat::EXCEL:
                return ".xlsx";
            case ExportFormat::PARQUET:
                return ".parquet";
            case ExportFormat::FEATHER:
                return ".feather";
            default:
                return ".txt";
            }
        }

        std::string ExportFactory::get_format_name(ExportFormat format)
        {
            switch (format)
            {
            case ExportFormat::CSV:
                return "CSV";
            case ExportFormat::JSON:
                return "JSON";
            case ExportFormat::XML:
                return "XML";
            case ExportFormat::EXCEL:
                return "Excel";
            case ExportFormat::PARQUET:
                return "Parquet";
            case ExportFormat::FEATHER:
                return "Feather";
            default:
                return "Unknown";
            }
        }

        // CSVExporter implementation
        bool CSVExporter::export_market_data(const MarketDataSeries &series, const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            // Write metadata
            if (!config.metadata.empty())
            {
                write_metadata(file, config.metadata);
                file << "\n";
            }

            // Write headers
            if (config.include_headers)
            {
                std::vector<std::string> headers = {"Timestamp", "Open", "High", "Low", "Close", "Volume"};
                write_headers(file, headers, config.delimiter);
            }

            // Write data
            for (const auto &point : series.data())
            {
                file << format_timestamp(point.timestamp) << config.delimiter;
                file << format_number(point.open) << config.delimiter;
                file << format_number(point.high) << config.delimiter;
                file << format_number(point.low) << config.delimiter;
                file << format_number(point.close) << config.delimiter;
                file << point.volume << "\n";
            }

            file.close();
            return true;
        }

        bool CSVExporter::export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            // Write headers
            if (config.include_headers)
            {
                std::vector<std::string> headers = {"Index", "SMA_20", "SMA_50", "EMA_12", "EMA_26", "RSI", "MACD"};
                write_headers(file, headers, config.delimiter);
            }

            // Find the maximum length
            size_t max_length = std::max({indicators.sma_20.size(), indicators.sma_50.size(),
                                          indicators.ema_12.size(), indicators.ema_26.size(),
                                          indicators.rsi.size(), indicators.macd.size()});

            // Write data
            for (size_t i = 0; i < max_length; ++i)
            {
                file << i << config.delimiter;
                file << (i < indicators.sma_20.size() ? format_number(indicators.sma_20[i]) : "") << config.delimiter;
                file << (i < indicators.sma_50.size() ? format_number(indicators.sma_50[i]) : "") << config.delimiter;
                file << (i < indicators.ema_12.size() ? format_number(indicators.ema_12[i]) : "") << config.delimiter;
                file << (i < indicators.ema_26.size() ? format_number(indicators.ema_26[i]) : "") << config.delimiter;
                file << (i < indicators.rsi.size() ? format_number(indicators.rsi[i]) : "") << config.delimiter;
                file << (i < indicators.macd.size() ? format_number(indicators.macd[i]) : "") << "\n";
            }

            file.close();
            return true;
        }

        bool CSVExporter::export_chart_data(const std::vector<CandlestickPoint> &data,
                                            const std::vector<IndicatorOverlay> &indicators,
                                            const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            // Write headers
            if (config.include_headers)
            {
                std::vector<std::string> headers = {"Timestamp", "Open", "High", "Low", "Close", "Volume"};
                write_headers(file, headers, config.delimiter);
            }

            // Write candlestick data
            for (const auto &point : data)
            {
                file << format_timestamp(point.timestamp) << config.delimiter;
                file << format_number(point.open) << config.delimiter;
                file << format_number(point.high) << config.delimiter;
                file << format_number(point.low) << config.delimiter;
                file << format_number(point.close) << config.delimiter;
                file << point.volume << "\n";
            }

            file.close();
            return true;
        }

        bool CSVExporter::export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                                  const std::vector<ChartPoint> &drawdown_data,
                                                  const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            // Write headers
            if (config.include_headers)
            {
                std::vector<std::string> headers = {"Index", "P&L", "Drawdown"};
                write_headers(file, headers, config.delimiter);
            }

            // Find the maximum length
            size_t max_length = std::max(pnl_data.size(), drawdown_data.size());

            // Write data
            for (size_t i = 0; i < max_length; ++i)
            {
                file << i << config.delimiter;
                file << (i < pnl_data.size() ? format_number(pnl_data[i].y) : "") << config.delimiter;
                file << (i < drawdown_data.size() ? format_number(drawdown_data[i].y) : "") << "\n";
            }

            file.close();
            return true;
        }

        bool CSVExporter::export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                                const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            // Write headers
            if (config.include_headers)
            {
                std::vector<std::string> headers = {"Symbol", "Quantity"};
                write_headers(file, headers, config.delimiter);
            }

            // Write data
            for (const auto &position : positions)
            {
                file << escape_csv_field(position.first, config.delimiter) << config.delimiter;
                file << format_number(position.second) << "\n";
            }

            file.close();
            return true;
        }

        bool CSVExporter::validate_config(const ExportConfig &config)
        {
            return !config.filename.empty() && !config.delimiter.empty();
        }

        std::vector<ExportFormat> CSVExporter::supported_formats() const
        {
            return {ExportFormat::CSV};
        }

        // Helper methods for CSVExporter
        std::string CSVExporter::escape_csv_field(const std::string &field, const std::string &delimiter)
        {
            if (field.find(delimiter) != std::string::npos ||
                field.find('"') != std::string::npos ||
                field.find('\n') != std::string::npos)
            {
                std::ostringstream oss;
                oss << "\"";
                for (char c : field)
                {
                    if (c == '"')
                        oss << "\"\"";
                    else
                        oss << c;
                }
                oss << "\"";
                return oss.str();
            }
            return field;
        }

        std::string CSVExporter::format_timestamp(const std::chrono::system_clock::time_point &timestamp)
        {
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        std::string CSVExporter::format_number(double value, int precision)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }

        void CSVExporter::write_headers(std::ofstream &file, const std::vector<std::string> &headers, const std::string &delimiter)
        {
            for (size_t i = 0; i < headers.size(); ++i)
            {
                file << headers[i];
                if (i < headers.size() - 1)
                    file << delimiter;
            }
            file << "\n";
        }

        void CSVExporter::write_metadata(std::ofstream &file, const std::map<std::string, std::string> &metadata)
        {
            for (const auto &[key, value] : metadata)
            {
                file << "# " << key << ": " << value << "\n";
            }
        }

        // JSONExporter implementation
        bool JSONExporter::export_market_data(const MarketDataSeries &series, const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            file << "{\n";
            file << "  \"metadata\": {\n";
            file << "    \"symbol\": \"" << series.symbol() << "\",\n";
            file << "    \"data_points\": " << series.size() << ",\n";
            file << "    \"export_time\": \"" << format_timestamp(std::chrono::system_clock::now()) << "\"\n";
            file << "  },\n";
            file << "  \"data\": [\n";

            for (size_t i = 0; i < series.size(); ++i)
            {
                const auto &point = series.data()[i];
                file << "    " << serialize_market_data_point(point);
                if (i < series.size() - 1)
                    file << ",";
                file << "\n";
            }

            file << "  ]\n";
            file << "}";
            file.close();
            return true;
        }

        bool JSONExporter::export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            file << "{\n";
            file << "  \"indicators\": {\n";
            file << "    \"sma_20\": [";
            for (size_t i = 0; i < indicators.sma_20.size(); ++i)
            {
                file << format_number(indicators.sma_20[i]);
                if (i < indicators.sma_20.size() - 1)
                    file << ", ";
            }
            file << "],\n";

            file << "    \"sma_50\": [";
            for (size_t i = 0; i < indicators.sma_50.size(); ++i)
            {
                file << format_number(indicators.sma_50[i]);
                if (i < indicators.sma_50.size() - 1)
                    file << ", ";
            }
            file << "],\n";

            file << "    \"rsi\": [";
            for (size_t i = 0; i < indicators.rsi.size(); ++i)
            {
                file << format_number(indicators.rsi[i]);
                if (i < indicators.rsi.size() - 1)
                    file << ", ";
            }
            file << "]\n";

            file << "  }\n";
            file << "}";
            file.close();
            return true;
        }

        bool JSONExporter::export_chart_data(const std::vector<CandlestickPoint> &data,
                                             const std::vector<IndicatorOverlay> &indicators,
                                             const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            file << "{\n";
            file << "  \"candlesticks\": [\n";

            for (size_t i = 0; i < data.size(); ++i)
            {
                file << "    " << serialize_candlestick_point(data[i]);
                if (i < data.size() - 1)
                    file << ",";
                file << "\n";
            }

            file << "  ],\n";
            file << "  \"indicators\": [\n";

            for (size_t i = 0; i < indicators.size(); ++i)
            {
                const auto &indicator = indicators[i];
                file << "    {\n";
                file << "      \"name\": \"" << indicator.name << "\",\n";
                file << "      \"color\": \"" << indicator.color << "\",\n";
                file << "      \"points\": [\n";

                for (size_t j = 0; j < indicator.points.size(); ++j)
                {
                    file << "        " << serialize_chart_point(indicator.points[j]);
                    if (j < indicator.points.size() - 1)
                        file << ",";
                    file << "\n";
                }

                file << "      ]\n";
                file << "    }";
                if (i < indicators.size() - 1)
                    file << ",";
                file << "\n";
            }

            file << "  ]\n";
            file << "}";
            file.close();
            return true;
        }

        bool JSONExporter::export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                                   const std::vector<ChartPoint> &drawdown_data,
                                                   const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            file << "{\n";
            file << "  \"pnl\": [\n";
            for (size_t i = 0; i < pnl_data.size(); ++i)
            {
                file << "    " << serialize_chart_point(pnl_data[i]);
                if (i < pnl_data.size() - 1)
                    file << ",";
                file << "\n";
            }
            file << "  ],\n";

            file << "  \"drawdown\": [\n";
            for (size_t i = 0; i < drawdown_data.size(); ++i)
            {
                file << "    " << serialize_chart_point(drawdown_data[i]);
                if (i < drawdown_data.size() - 1)
                    file << ",";
                file << "\n";
            }
            file << "  ]\n";
            file << "}";
            file.close();
            return true;
        }

        bool JSONExporter::export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                                 const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            file << "{\n";
            file << "  \"positions\": [\n";

            for (size_t i = 0; i < positions.size(); ++i)
            {
                const auto &position = positions[i];
                file << "    {\n";
                file << "      \"symbol\": \"" << position.first << "\",\n";
                file << "      \"quantity\": " << format_number(position.second) << "\n";
                file << "    }";
                if (i < positions.size() - 1)
                    file << ",";
                file << "\n";
            }

            file << "  ]\n";
            file << "}";
            file.close();
            return true;
        }

        bool JSONExporter::validate_config(const ExportConfig &config)
        {
            return !config.filename.empty();
        }

        std::vector<ExportFormat> JSONExporter::supported_formats() const
        {
            return {ExportFormat::JSON};
        }

        // Helper methods for JSONExporter
        std::string JSONExporter::format_timestamp(const std::chrono::system_clock::time_point &timestamp)
        {
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        std::string JSONExporter::format_number(double value, int precision)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }

        std::string JSONExporter::serialize_market_data_point(const MarketDataPoint &point)
        {
            std::ostringstream oss;
            oss << "{\n";
            oss << "      \"timestamp\": \"" << format_timestamp(point.timestamp) << "\",\n";
            oss << "      \"open\": " << format_number(point.open) << ",\n";
            oss << "      \"high\": " << format_number(point.high) << ",\n";
            oss << "      \"low\": " << format_number(point.low) << ",\n";
            oss << "      \"close\": " << format_number(point.close) << ",\n";
            oss << "      \"volume\": " << point.volume << "\n";
            oss << "    }";
            return oss.str();
        }

        std::string JSONExporter::serialize_chart_point(const ChartPoint &point)
        {
            std::ostringstream oss;
            oss << "{\"x\": " << point.x << ", \"y\": " << format_number(point.y) << "}";
            return oss.str();
        }

        std::string JSONExporter::serialize_candlestick_point(const CandlestickPoint &point)
        {
            std::ostringstream oss;
            oss << "{\n";
            oss << "      \"timestamp\": \"" << format_timestamp(point.timestamp) << "\",\n";
            oss << "      \"open\": " << format_number(point.open) << ",\n";
            oss << "      \"high\": " << format_number(point.high) << ",\n";
            oss << "      \"low\": " << format_number(point.low) << ",\n";
            oss << "      \"close\": " << format_number(point.close) << ",\n";
            oss << "      \"volume\": " << point.volume << ",\n";
            oss << "      \"is_green\": " << (point.is_green ? "true" : "false") << "\n";
            oss << "    }";
            return oss.str();
        }

        // XMLExporter implementation (simplified)
        bool XMLExporter::export_market_data(const MarketDataSeries &series, const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;

            std::string output_path = ExportUtils::get_output_path(config.filename);
            std::ofstream file(output_path);
            if (!file.is_open())
                return false;

            write_xml_header(file, "market_data");
            file << "  <symbol>" << escape_xml(series.symbol()) << "</symbol>\n";
            file << "  <data_points>" << series.size() << "</data_points>\n";

            for (const auto &point : series.data())
            {
                file << "  <point>\n";
                file << "    <timestamp>" << format_timestamp(point.timestamp) << "</timestamp>\n";
                file << "    <open>" << format_number(point.open) << "</open>\n";
                file << "    <high>" << format_number(point.high) << "</high>\n";
                file << "    <low>" << format_number(point.low) << "</low>\n";
                file << "    <close>" << format_number(point.close) << "</close>\n";
                file << "    <volume>" << point.volume << "</volume>\n";
                file << "  </point>\n";
            }

            write_xml_footer(file, "market_data");
            file.close();
            return true;
        }

        bool XMLExporter::export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;
            // Simplified implementation
            return true;
        }

        bool XMLExporter::export_chart_data(const std::vector<CandlestickPoint> &data,
                                            const std::vector<IndicatorOverlay> &indicators,
                                            const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;
            // Simplified implementation
            return true;
        }

        bool XMLExporter::export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                                  const std::vector<ChartPoint> &drawdown_data,
                                                  const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;
            // Simplified implementation
            return true;
        }

        bool XMLExporter::export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                                const ExportConfig &config)
        {
            if (!validate_config(config))
                return false;
            // Simplified implementation
            return true;
        }

        bool XMLExporter::validate_config(const ExportConfig &config)
        {
            return !config.filename.empty();
        }

        std::vector<ExportFormat> XMLExporter::supported_formats() const
        {
            return {ExportFormat::XML};
        }

        // Helper methods for XMLExporter
        std::string XMLExporter::format_timestamp(const std::chrono::system_clock::time_point &timestamp)
        {
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        std::string XMLExporter::format_number(double value, int precision)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }

        std::string XMLExporter::escape_xml(const std::string &text)
        {
            std::string result = text;
            // Simple XML escaping
            size_t pos = 0;
            while ((pos = result.find("&", pos)) != std::string::npos)
            {
                result.replace(pos, 1, "&amp;");
                pos += 5;
            }
            pos = 0;
            while ((pos = result.find("<", pos)) != std::string::npos)
            {
                result.replace(pos, 1, "&lt;");
                pos += 4;
            }
            pos = 0;
            while ((pos = result.find(">", pos)) != std::string::npos)
            {
                result.replace(pos, 1, "&gt;");
                pos += 4;
            }
            return result;
        }

        void XMLExporter::write_xml_header(std::ofstream &file, const std::string &root_element)
        {
            file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            file << "<" << root_element << ">\n";
        }

        void XMLExporter::write_xml_footer(std::ofstream &file, const std::string &root_element)
        {
            file << "</" << root_element << ">\n";
        }

        // ExcelExporter implementation (simplified - uses CSV format)
        bool ExcelExporter::export_market_data(const MarketDataSeries &series, const ExportConfig &config)
        {
            // For now, just use CSV format
            CSVExporter csv_exporter;
            return csv_exporter.export_market_data(series, config);
        }

        bool ExcelExporter::export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config)
        {
            CSVExporter csv_exporter;
            return csv_exporter.export_indicators(indicators, config);
        }

        bool ExcelExporter::export_chart_data(const std::vector<CandlestickPoint> &data,
                                              const std::vector<IndicatorOverlay> &indicators,
                                              const ExportConfig &config)
        {
            CSVExporter csv_exporter;
            return csv_exporter.export_chart_data(data, indicators, config);
        }

        bool ExcelExporter::export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                                    const std::vector<ChartPoint> &drawdown_data,
                                                    const ExportConfig &config)
        {
            CSVExporter csv_exporter;
            return csv_exporter.export_performance_data(pnl_data, drawdown_data, config);
        }

        bool ExcelExporter::export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                                  const ExportConfig &config)
        {
            CSVExporter csv_exporter;
            return csv_exporter.export_portfolio_data(positions, config);
        }

        bool ExcelExporter::validate_config(const ExportConfig &config)
        {
            return !config.filename.empty();
        }

        std::vector<ExportFormat> ExcelExporter::supported_formats() const
        {
            return {ExportFormat::EXCEL};
        }

        // Helper methods for ExcelExporter
        std::string ExcelExporter::format_timestamp(const std::chrono::system_clock::time_point &timestamp)
        {
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        std::string ExcelExporter::format_number(double value, int precision)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }

        void ExcelExporter::write_excel_header(std::ofstream &file)
        {
            // Simplified Excel header
            file << "Excel Export\n";
        }

        void ExcelExporter::write_excel_footer(std::ofstream &file)
        {
            // Simplified Excel footer
            file << "End of Export\n";
        }

        // BatchExporter implementation
        void BatchExporter::add_exporter(std::unique_ptr<DataExporter> exporter)
        {
            exporters_.push_back(std::move(exporter));
        }

        void BatchExporter::add_export_config(const std::string &name, const ExportConfig &config)
        {
            export_configs_[name] = config;
        }

        bool BatchExporter::export_market_data_batch(const MarketDataSeries &series)
        {
            bool success = true;
            for (auto &exporter : exporters_)
            {
                for (const auto &[name, config] : export_configs_)
                {
                    if (!exporter->export_market_data(series, config))
                    {
                        success = false;
                    }
                }
            }
            return success;
        }

        bool BatchExporter::export_indicators_batch(const TechnicalIndicators &indicators)
        {
            bool success = true;
            for (auto &exporter : exporters_)
            {
                for (const auto &[name, config] : export_configs_)
                {
                    if (!exporter->export_indicators(indicators, config))
                    {
                        success = false;
                    }
                }
            }
            return success;
        }

        bool BatchExporter::export_chart_data_batch(const std::vector<CandlestickPoint> &data,
                                                    const std::vector<IndicatorOverlay> &indicators)
        {
            bool success = true;
            for (auto &exporter : exporters_)
            {
                for (const auto &[name, config] : export_configs_)
                {
                    if (!exporter->export_chart_data(data, indicators, config))
                    {
                        success = false;
                    }
                }
            }
            return success;
        }

        bool BatchExporter::export_performance_data_batch(const std::vector<ChartPoint> &pnl_data,
                                                          const std::vector<ChartPoint> &drawdown_data)
        {
            bool success = true;
            for (auto &exporter : exporters_)
            {
                for (const auto &[name, config] : export_configs_)
                {
                    if (!exporter->export_performance_data(pnl_data, drawdown_data, config))
                    {
                        success = false;
                    }
                }
            }
            return success;
        }

        bool BatchExporter::export_portfolio_data_batch(const std::vector<std::pair<std::string, double>> &positions)
        {
            bool success = true;
            for (auto &exporter : exporters_)
            {
                for (const auto &[name, config] : export_configs_)
                {
                    if (!exporter->export_portfolio_data(positions, config))
                    {
                        success = false;
                    }
                }
            }
            return success;
        }

        void BatchExporter::clear_configs()
        {
            export_configs_.clear();
        }

        std::map<std::string, bool> BatchExporter::get_export_status() const
        {
            // Simplified implementation
            std::map<std::string, bool> status;
            for (const auto &[name, config] : export_configs_)
            {
                status[name] = true; // Assume success for demo
            }
            return status;
        }

        // ExportUtils implementation
        std::string ExportUtils::generate_filename(const std::string &base_name, const std::string &extension)
        {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::ostringstream oss;
            oss << base_name << "_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << extension;
            return oss.str();
        }

        std::string ExportUtils::get_output_path(const std::string &filename)
        {
            // Ensure output directory exists
            ensure_directory("output");
            
            // Check if filename already contains output directory
            std::string filename_lower = filename;
            std::transform(filename_lower.begin(), filename_lower.end(), filename_lower.begin(), ::tolower);
            if (filename_lower.find("output/") == 0 || 
                filename_lower.find("output\\") == 0)
            {
                // Already contains output path, return as is
                return filename;
            }
            
            // Use filesystem::path for cross-platform path handling
            std::filesystem::path output_dir("output");
            std::filesystem::path file_path(filename);
            std::filesystem::path result = output_dir / file_path;
            return result.string();
        }

        bool ExportUtils::ensure_directory(const std::string &path)
        {
            try
            {
                std::filesystem::create_directories(path);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        bool ExportUtils::validate_file_path(const std::string &path)
        {
            try
            {
                std::filesystem::path fs_path(path);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        size_t ExportUtils::get_file_size(const std::string &filename)
        {
            try
            {
                return std::filesystem::file_size(filename);
            }
            catch (...)
            {
                return 0;
            }
        }

        bool ExportUtils::compress_file(const std::string &input_file, const std::string &output_file)
        {
            // Simplified implementation - would use compression library in real implementation
            return false;
        }

        std::string ExportUtils::format_file_size(size_t bytes)
        {
            const char *units[] = {"B", "KB", "MB", "GB", "TB"};
            int unit_index = 0;
            double size = static_cast<double>(bytes);

            while (size >= 1024.0 && unit_index < 4)
            {
                size /= 1024.0;
                unit_index++;
            }

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << size << " " << units[unit_index];
            return oss.str();
        }

        std::vector<std::pair<ExportFormat, std::string>> ExportUtils::get_supported_formats()
        {
            return {
                {ExportFormat::CSV, "Comma-Separated Values"},
                {ExportFormat::JSON, "JavaScript Object Notation"},
                {ExportFormat::XML, "Extensible Markup Language"},
                {ExportFormat::EXCEL, "Microsoft Excel"},
                {ExportFormat::PARQUET, "Apache Parquet"},
                {ExportFormat::FEATHER, "Apache Arrow Feather"}};
        }

        ExportConfig ExportUtils::parse_config_from_file(const std::string &config_file)
        {
            // Simplified implementation
            ExportConfig config("default.csv", ExportFormat::CSV);
            return config;
        }

        bool ExportUtils::save_config_to_file(const ExportConfig &config, const std::string &config_file)
        {
            // Simplified implementation
            return true;
        }

    } // namespace visualization
} // namespace trading
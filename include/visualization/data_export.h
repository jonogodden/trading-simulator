#pragma once

#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <map>
#include <chrono>

#include "../data/market_data.h"
#include "../data/data_processor.h"
#include "chart_renderer.h"

namespace trading
{
    namespace visualization
    {

        // Export formats
        enum class ExportFormat
        {
            CSV,
            JSON,
            XML,
            EXCEL,
            PARQUET,
            FEATHER
        };

        // Export configuration
        struct ExportConfig
        {
            ExportFormat format = ExportFormat::CSV;
            std::string filename;
            std::string delimiter = ",";
            bool include_headers = true;
            bool include_timestamps = true;
            std::vector<std::string> columns;
            std::map<std::string, std::string> metadata;

            ExportConfig() = default;
            ExportConfig(const std::string &fname, ExportFormat fmt = ExportFormat::CSV)
                : filename(fname), format(fmt) {}
        };

        // Data export interface
        class DataExporter
        {
        public:
            virtual ~DataExporter() = default;

            // Export market data
            virtual bool export_market_data(const MarketDataSeries &series, const ExportConfig &config) = 0;

            // Export technical indicators
            virtual bool export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config) = 0;

            // Export chart data
            virtual bool export_chart_data(const std::vector<CandlestickPoint> &data,
                                           const std::vector<IndicatorOverlay> &indicators,
                                           const ExportConfig &config) = 0;

            // Export performance data
            virtual bool export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                                 const std::vector<ChartPoint> &drawdown_data,
                                                 const ExportConfig &config) = 0;

            // Export portfolio data
            virtual bool export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                               const ExportConfig &config) = 0;

            // Validate export configuration
            virtual bool validate_config(const ExportConfig &config) = 0;

            // Get supported formats
            virtual std::vector<ExportFormat> supported_formats() const = 0;
        };

        // CSV Exporter
        class CSVExporter : public DataExporter
        {
        private:
            // Helper methods
            std::string escape_csv_field(const std::string &field, const std::string &delimiter);
            std::string format_timestamp(const std::chrono::system_clock::time_point &timestamp);
            std::string format_number(double value, int precision = 6);
            void write_headers(std::ofstream &file, const std::vector<std::string> &headers, const std::string &delimiter);
            void write_metadata(std::ofstream &file, const std::map<std::string, std::string> &metadata);

        public:
            CSVExporter() = default;
            ~CSVExporter() override = default;

            bool export_market_data(const MarketDataSeries &series, const ExportConfig &config) override;
            bool export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config) override;
            bool export_chart_data(const std::vector<CandlestickPoint> &data,
                                   const std::vector<IndicatorOverlay> &indicators,
                                   const ExportConfig &config) override;
            bool export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                         const std::vector<ChartPoint> &drawdown_data,
                                         const ExportConfig &config) override;
            bool export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                       const ExportConfig &config) override;
            bool validate_config(const ExportConfig &config) override;
            std::vector<ExportFormat> supported_formats() const override;
        };

        // JSON Exporter
        class JSONExporter : public DataExporter
        {
        private:
            // Helper methods
            std::string format_timestamp(const std::chrono::system_clock::time_point &timestamp);
            std::string format_number(double value, int precision = 6);
            std::string serialize_market_data_point(const MarketDataPoint &point);
            std::string serialize_chart_point(const ChartPoint &point);
            std::string serialize_candlestick_point(const CandlestickPoint &point);

        public:
            JSONExporter() = default;
            ~JSONExporter() override = default;

            bool export_market_data(const MarketDataSeries &series, const ExportConfig &config) override;
            bool export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config) override;
            bool export_chart_data(const std::vector<CandlestickPoint> &data,
                                   const std::vector<IndicatorOverlay> &indicators,
                                   const ExportConfig &config) override;
            bool export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                         const std::vector<ChartPoint> &drawdown_data,
                                         const ExportConfig &config) override;
            bool export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                       const ExportConfig &config) override;
            bool validate_config(const ExportConfig &config) override;
            std::vector<ExportFormat> supported_formats() const override;
        };

        // XML Exporter
        class XMLExporter : public DataExporter
        {
        private:
            // Helper methods
            std::string format_timestamp(const std::chrono::system_clock::time_point &timestamp);
            std::string format_number(double value, int precision = 6);
            std::string escape_xml(const std::string &text);
            void write_xml_header(std::ofstream &file, const std::string &root_element);
            void write_xml_footer(std::ofstream &file, const std::string &root_element);

        public:
            XMLExporter() = default;
            ~XMLExporter() override = default;

            bool export_market_data(const MarketDataSeries &series, const ExportConfig &config) override;
            bool export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config) override;
            bool export_chart_data(const std::vector<CandlestickPoint> &data,
                                   const std::vector<IndicatorOverlay> &indicators,
                                   const ExportConfig &config) override;
            bool export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                         const std::vector<ChartPoint> &drawdown_data,
                                         const ExportConfig &config) override;
            bool export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                       const ExportConfig &config) override;
            bool validate_config(const ExportConfig &config) override;
            std::vector<ExportFormat> supported_formats() const override;
        };

        // Excel Exporter (basic implementation using CSV format)
        class ExcelExporter : public DataExporter
        {
        private:
            // Helper methods
            std::string format_timestamp(const std::chrono::system_clock::time_point &timestamp);
            std::string format_number(double value, int precision = 6);
            void write_excel_header(std::ofstream &file);
            void write_excel_footer(std::ofstream &file);

        public:
            ExcelExporter() = default;
            ~ExcelExporter() override = default;

            bool export_market_data(const MarketDataSeries &series, const ExportConfig &config) override;
            bool export_indicators(const TechnicalIndicators &indicators, const ExportConfig &config) override;
            bool export_chart_data(const std::vector<CandlestickPoint> &data,
                                   const std::vector<IndicatorOverlay> &indicators,
                                   const ExportConfig &config) override;
            bool export_performance_data(const std::vector<ChartPoint> &pnl_data,
                                         const std::vector<ChartPoint> &drawdown_data,
                                         const ExportConfig &config) override;
            bool export_portfolio_data(const std::vector<std::pair<std::string, double>> &positions,
                                       const ExportConfig &config) override;
            bool validate_config(const ExportConfig &config) override;
            std::vector<ExportFormat> supported_formats() const override;
        };

        // Export factory
        class ExportFactory
        {
        public:
            static std::unique_ptr<DataExporter> create_exporter(ExportFormat format);
            static std::string get_file_extension(ExportFormat format);
            static std::string get_format_name(ExportFormat format);
        };

        // Batch export manager
        class BatchExporter
        {
        private:
            std::vector<std::unique_ptr<DataExporter>> exporters_;
            std::map<std::string, ExportConfig> export_configs_;

        public:
            BatchExporter() = default;
            ~BatchExporter() = default;

            // Add exporter
            void add_exporter(std::unique_ptr<DataExporter> exporter);

            // Add export configuration
            void add_export_config(const std::string &name, const ExportConfig &config);

            // Batch export market data
            bool export_market_data_batch(const MarketDataSeries &series);

            // Batch export indicators
            bool export_indicators_batch(const TechnicalIndicators &indicators);

            // Batch export chart data
            bool export_chart_data_batch(const std::vector<CandlestickPoint> &data,
                                         const std::vector<IndicatorOverlay> &indicators);

            // Batch export performance data
            bool export_performance_data_batch(const std::vector<ChartPoint> &pnl_data,
                                               const std::vector<ChartPoint> &drawdown_data);

            // Batch export portfolio data
            bool export_portfolio_data_batch(const std::vector<std::pair<std::string, double>> &positions);

            // Clear all configurations
            void clear_configs();

            // Get export status
            std::map<std::string, bool> get_export_status() const;
        };

        // Data export utilities
        namespace ExportUtils
        {

            // Generate filename with timestamp
            std::string generate_filename(const std::string &base_name, const std::string &extension);

            // Create directory if it doesn't exist
            bool ensure_directory(const std::string &path);

            // Validate file path
            bool validate_file_path(const std::string &path);

            // Get file size
            size_t get_file_size(const std::string &filename);

            // Compress file (if supported)
            bool compress_file(const std::string &input_file, const std::string &output_file);

            // Format file size for display
            std::string format_file_size(size_t bytes);

            // Get supported export formats
            std::vector<std::pair<ExportFormat, std::string>> get_supported_formats();

            // Parse export configuration from file
            ExportConfig parse_config_from_file(const std::string &config_file);

            // Save export configuration to file
            bool save_config_to_file(const ExportConfig &config, const std::string &config_file);

        } // namespace ExportUtils

    } // namespace visualization
} // namespace trading
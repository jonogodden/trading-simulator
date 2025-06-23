#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <chrono>

#include "../data/market_data.h"
#include "../data/data_processor.h"

namespace trading
{
    namespace visualization
    {

        // Forward declarations
        class ChartCanvas;
        class ChartStyle;

        // Chart types
        enum class ChartType
        {
            CANDLESTICK,
            LINE,
            BAR,
            AREA,
            SCATTER
        };

        // Chart configuration
        struct ChartConfig
        {
            int width = 800;
            int height = 600;
            std::string title = "Trading Chart";
            std::string x_axis_label = "Time";
            std::string y_axis_label = "Price";
            bool show_grid = true;
            bool show_legend = true;
            bool show_volume = true;
            std::string background_color = "#1e1e1e";
            std::string grid_color = "#333333";
            std::string text_color = "#ffffff";
        };

        // Chart data point for rendering
        struct ChartPoint
        {
            double x; // Time or index
            double y; // Value
            std::string label;
            std::string color;

            ChartPoint(double x_val, double y_val, const std::string &lbl = "", const std::string &clr = "")
                : x(x_val), y(y_val), label(lbl), color(clr) {}
        };

        // Chart series data
        struct ChartSeries
        {
            std::string name;
            std::vector<ChartPoint> points;
            ChartType type;
            std::string color;
            bool visible = true;

            ChartSeries(const std::string &n, ChartType t, const std::string &c = "")
                : name(n), type(t), color(c) {}
        };

        // Candlestick data point
        struct CandlestickPoint
        {
            std::chrono::system_clock::time_point timestamp;
            double open;
            double high;
            double low;
            double close;
            uint64_t volume;
            bool is_green; // true if close > open

            CandlestickPoint(const MarketDataPoint &point)
                : timestamp(point.timestamp), open(point.open), high(point.high), low(point.low), close(point.close), volume(point.volume), is_green(point.close > point.open) {}
        };

        // Technical indicator overlay
        struct IndicatorOverlay
        {
            std::string name;
            std::vector<ChartPoint> points;
            std::string color;
            double opacity = 0.7;
            bool visible = true;

            IndicatorOverlay(const std::string &n, const std::string &c = "")
                : name(n), color(c) {}
        };

        // Chart renderer interface
        class ChartRenderer
        {
        public:
            virtual ~ChartRenderer() = default;

            // Initialize the renderer
            virtual bool initialize(const ChartConfig &config) = 0;

            // Render candlestick chart
            virtual bool render_candlestick_chart(
                const std::vector<CandlestickPoint> &data,
                const std::vector<IndicatorOverlay> &indicators = {},
                const ChartConfig &config = ChartConfig{}) = 0;

            // Render line chart
            virtual bool render_line_chart(
                const std::vector<ChartSeries> &series,
                const ChartConfig &config = ChartConfig{}) = 0;

            // Render bar chart
            virtual bool render_bar_chart(
                const std::vector<ChartSeries> &series,
                const ChartConfig &config = ChartConfig{}) = 0;

            // Render technical indicators
            virtual bool render_indicators(
                const std::vector<IndicatorOverlay> &indicators,
                const ChartConfig &config = ChartConfig{}) = 0;

            // Export chart to file
            virtual bool export_to_file(const std::string &filename, const std::string &format = "png") = 0;

            // Get chart as data (for web rendering)
            virtual std::string get_chart_data(const std::string &format = "json") = 0;

            // Clear the chart
            virtual void clear() = 0;
        };

        // HTML/SVG Chart Renderer (for web-based visualization)
        class HTMLChartRenderer : public ChartRenderer
        {
        private:
            ChartConfig current_config_;
            std::vector<ChartSeries> current_series_;
            std::vector<IndicatorOverlay> current_indicators_;
            std::string current_chart_data_;

            // Helper methods
            std::string generate_svg_candlestick(const std::vector<CandlestickPoint> &data);
            std::string generate_svg_line(const std::vector<ChartSeries> &series);
            std::string generate_svg_bar(const std::vector<ChartSeries> &series);
            std::string generate_svg_indicators(const std::vector<IndicatorOverlay> &indicators);
            std::string generate_html_wrapper(const std::string &svg_content);
            std::string generate_json_data();

            // Utility functions
            std::string format_timestamp(const std::chrono::system_clock::time_point &timestamp);
            std::string format_number(double value, int precision = 2);
            std::string get_color_scheme(const std::string &base_color);

        public:
            HTMLChartRenderer() = default;
            ~HTMLChartRenderer() override = default;

            bool initialize(const ChartConfig &config) override;
            bool render_candlestick_chart(
                const std::vector<CandlestickPoint> &data,
                const std::vector<IndicatorOverlay> &indicators = {},
                const ChartConfig &config = ChartConfig{}) override;
            bool render_line_chart(
                const std::vector<ChartSeries> &series,
                const ChartConfig &config = ChartConfig{}) override;
            bool render_bar_chart(
                const std::vector<ChartSeries> &series,
                const ChartConfig &config = ChartConfig{}) override;
            bool render_indicators(
                const std::vector<IndicatorOverlay> &indicators,
                const ChartConfig &config = ChartConfig{}) override;
            bool export_to_file(const std::string &filename, const std::string &format = "html") override;
            std::string get_chart_data(const std::string &format = "json") override;
            void clear() override;
        };

        // Console Chart Renderer (for terminal-based visualization)
        class ConsoleChartRenderer : public ChartRenderer
        {
        private:
            ChartConfig current_config_;
            std::vector<ChartSeries> current_series_;

            // Console rendering helpers
            void render_candlestick_console(const std::vector<CandlestickPoint> &data);
            void render_line_console(const std::vector<ChartSeries> &series);
            void render_volume_console(const std::vector<CandlestickPoint> &data);
            void render_indicators_console(const std::vector<IndicatorOverlay> &indicators);

            // Utility functions
            std::string format_price(double price, int width = 8);
            std::string format_volume(uint64_t volume);
            std::string format_timestamp(const std::chrono::system_clock::time_point &timestamp);
            std::string get_candlestick_symbol(bool is_green, double body_ratio);

        public:
            ConsoleChartRenderer() = default;
            ~ConsoleChartRenderer() override = default;

            bool initialize(const ChartConfig &config) override;
            bool render_candlestick_chart(
                const std::vector<CandlestickPoint> &data,
                const std::vector<IndicatorOverlay> &indicators = {},
                const ChartConfig &config = ChartConfig{}) override;
            bool render_line_chart(
                const std::vector<ChartSeries> &series,
                const ChartConfig &config = ChartConfig{}) override;
            bool render_bar_chart(
                const std::vector<ChartSeries> &series,
                const ChartConfig &config = ChartConfig{}) override;
            bool render_indicators(
                const std::vector<IndicatorOverlay> &indicators,
                const ChartConfig &config = ChartConfig{}) override;
            bool export_to_file(const std::string &filename, const std::string &format = "txt") override;
            std::string get_chart_data(const std::string &format = "text") override;
            void clear() override;
        };

        // Chart factory for creating different renderers
        class ChartFactory
        {
        public:
            enum class RendererType
            {
                HTML,
                CONSOLE,
                // Future: PDF, PNG, etc.
            };

            static std::unique_ptr<ChartRenderer> create_renderer(RendererType type);
        };

    } // namespace visualization
} // namespace trading
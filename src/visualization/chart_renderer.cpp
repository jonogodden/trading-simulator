#include "visualization/chart_renderer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace trading
{
    namespace visualization
    {

        // ChartFactory implementation
        std::unique_ptr<ChartRenderer> ChartFactory::create_renderer(RendererType type)
        {
            switch (type)
            {
            case RendererType::HTML:
                return std::make_unique<HTMLChartRenderer>();
            case RendererType::CONSOLE:
                return std::make_unique<ConsoleChartRenderer>();
            default:
                return nullptr;
            }
        }

        // HTMLChartRenderer implementation
        bool HTMLChartRenderer::initialize(const ChartConfig &config)
        {
            current_config_ = config;
            current_chart_data_.clear();
            return true;
        }

        bool HTMLChartRenderer::render_candlestick_chart(
            const std::vector<CandlestickPoint> &data,
            const std::vector<IndicatorOverlay> &indicators,
            const ChartConfig &config)
        {

            if (config.width > 0)
                current_config_.width = config.width;
            if (config.height > 0)
                current_config_.height = config.height;

            std::string svg_content = generate_svg_candlestick(data);

            if (!indicators.empty())
            {
                svg_content += generate_svg_indicators(indicators);
            }

            current_chart_data_ = generate_html_wrapper(svg_content);
            return true;
        }

        bool HTMLChartRenderer::render_line_chart(
            const std::vector<ChartSeries> &series,
            const ChartConfig &config)
        {

            if (config.width > 0)
                current_config_.width = config.width;
            if (config.height > 0)
                current_config_.height = config.height;

            current_series_ = series;
            std::string svg_content = generate_svg_line(series);
            current_chart_data_ = generate_html_wrapper(svg_content);
            return true;
        }

        bool HTMLChartRenderer::render_bar_chart(
            const std::vector<ChartSeries> &series,
            const ChartConfig &config)
        {

            if (config.width > 0)
                current_config_.width = config.width;
            if (config.height > 0)
                current_config_.height = config.height;

            current_series_ = series;
            std::string svg_content = generate_svg_bar(series);
            current_chart_data_ = generate_html_wrapper(svg_content);
            return true;
        }

        bool HTMLChartRenderer::render_indicators(
            const std::vector<IndicatorOverlay> &indicators,
            const ChartConfig &config)
        {

            if (config.width > 0)
                current_config_.width = config.width;
            if (config.height > 0)
                current_config_.height = config.height;

            current_indicators_ = indicators;
            std::string svg_content = generate_svg_indicators(indicators);
            current_chart_data_ = generate_html_wrapper(svg_content);
            return true;
        }

        bool HTMLChartRenderer::export_to_file(const std::string &filename, const std::string &format)
        {
            if (current_chart_data_.empty())
            {
                return false;
            }

            std::ofstream file(filename);
            if (!file.is_open())
            {
                return false;
            }

            file << current_chart_data_;
            file.close();
            return true;
        }

        std::string HTMLChartRenderer::get_chart_data(const std::string &format)
        {
            if (format == "json")
            {
                return generate_json_data();
            }
            return current_chart_data_;
        }

        void HTMLChartRenderer::clear()
        {
            current_chart_data_.clear();
            current_series_.clear();
            current_indicators_.clear();
        }

        // Helper methods for HTMLChartRenderer
        std::string HTMLChartRenderer::generate_svg_candlestick(const std::vector<CandlestickPoint> &data)
        {
            if (data.empty())
                return "";

            std::ostringstream svg;

            // Calculate dimensions
            const int margin = 50;
            const int chart_width = current_config_.width - 2 * margin;
            const int chart_height = current_config_.height - 2 * margin;

            // Find price range
            double min_price = data[0].low;
            double max_price = data[0].high;
            for (const auto &point : data)
            {
                min_price = std::min(min_price, point.low);
                max_price = std::max(max_price, point.high);
            }

            const double price_range = max_price - min_price;
            const double price_padding = price_range * 0.1;

            // SVG header
            svg << "<svg width=\"" << current_config_.width << "\" height=\"" << current_config_.height << "\" ";
            svg << "style=\"background-color: " << current_config_.background_color << ";\">\n";

            // Grid
            if (current_config_.show_grid)
            {
                svg << "  <defs>\n";
                svg << "    <pattern id=\"grid\" width=\"40\" height=\"40\" patternUnits=\"userSpaceOnUse\">\n";
                svg << "      <path d=\"M 40 0 L 0 0 0 40\" fill=\"none\" stroke=\"" << current_config_.grid_color << "\" stroke-width=\"1\" opacity=\"0.3\"/>\n";
                svg << "    </pattern>\n";
                svg << "  </defs>\n";
                svg << "  <rect width=\"100%\" height=\"100%\" fill=\"url(#grid)\"/>\n";
            }

            // Candlesticks
            const double candle_width = std::max(2.0, static_cast<double>(chart_width) / data.size() * 0.8);
            const double spacing = static_cast<double>(chart_width) / data.size();

            for (size_t i = 0; i < data.size(); ++i)
            {
                const auto &point = data[i];
                const double x = margin + i * spacing + spacing / 2;

                // Price to y coordinate conversion
                const double y_high = margin + (max_price + price_padding - point.high) / (price_range + 2 * price_padding) * chart_height;
                const double y_low = margin + (max_price + price_padding - point.low) / (price_range + 2 * price_padding) * chart_height;
                const double y_open = margin + (max_price + price_padding - point.open) / (price_range + 2 * price_padding) * chart_height;
                const double y_close = margin + (max_price + price_padding - point.close) / (price_range + 2 * price_padding) * chart_height;

                // Wick
                svg << "  <line x1=\"" << x << "\" y1=\"" << y_high << "\" x2=\"" << x << "\" y2=\"" << y_low << "\" ";
                svg << "stroke=\"" << (point.is_green ? "#00ff00" : "#ff0000") << "\" stroke-width=\"1\"/>\n";

                // Body
                const double body_top = std::min(y_open, y_close);
                const double body_height = std::abs(y_close - y_open);
                const double body_left = x - candle_width / 2;

                svg << "  <rect x=\"" << body_left << "\" y=\"" << body_top << "\" ";
                svg << "width=\"" << candle_width << "\" height=\"" << body_height << "\" ";
                svg << "fill=\"" << (point.is_green ? "#00ff00" : "#ff0000") << "\" ";
                svg << "stroke=\"" << (point.is_green ? "#00cc00" : "#cc0000") << "\" stroke-width=\"1\"/>\n";
            }

            // Title
            svg << "  <text x=\"" << current_config_.width / 2 << "\" y=\"25\" ";
            svg << "text-anchor=\"middle\" fill=\"" << current_config_.text_color << "\" font-size=\"16\" font-weight=\"bold\">";
            svg << current_config_.title << "</text>\n";

            // Axis labels
            svg << "  <text x=\"" << current_config_.width / 2 << "\" y=\"" << current_config_.height - 10 << "\" ";
            svg << "text-anchor=\"middle\" fill=\"" << current_config_.text_color << "\" font-size=\"12\">";
            svg << current_config_.x_axis_label << "</text>\n";

            svg << "  <text x=\"10\" y=\"" << current_config_.height / 2 << "\" ";
            svg << "text-anchor=\"middle\" fill=\"" << current_config_.text_color << "\" font-size=\"12\" ";
            svg << "transform=\"rotate(-90, 10, " << current_config_.height / 2 << ")\">";
            svg << current_config_.y_axis_label << "</text>\n";

            svg << "</svg>";
            return svg.str();
        }

        std::string HTMLChartRenderer::generate_svg_line(const std::vector<ChartSeries> &series)
        {
            if (series.empty() || series[0].points.empty())
                return "";

            std::ostringstream svg;

            // Calculate dimensions
            const int margin = 50;
            const int chart_width = current_config_.width - 2 * margin;
            const int chart_height = current_config_.height - 2 * margin;

            // Find data range
            double min_x = series[0].points[0].x;
            double max_x = series[0].points[0].x;
            double min_y = series[0].points[0].y;
            double max_y = series[0].points[0].y;

            for (const auto &s : series)
            {
                for (const auto &point : s.points)
                {
                    min_x = std::min(min_x, point.x);
                    max_x = std::max(max_x, point.x);
                    min_y = std::min(min_y, point.y);
                    max_y = std::max(max_y, point.y);
                }
            }

            const double x_range = max_x - min_x;
            const double y_range = max_y - min_y;
            const double x_padding = x_range * 0.05;
            const double y_padding = y_range * 0.1;

            // SVG header
            svg << "<svg width=\"" << current_config_.width << "\" height=\"" << current_config_.height << "\" ";
            svg << "style=\"background-color: " << current_config_.background_color << ";\">\n";

            // Grid
            if (current_config_.show_grid)
            {
                svg << "  <defs>\n";
                svg << "    <pattern id=\"grid\" width=\"40\" height=\"40\" patternUnits=\"userSpaceOnUse\">\n";
                svg << "      <path d=\"M 40 0 L 0 0 0 40\" fill=\"none\" stroke=\"" << current_config_.grid_color << "\" stroke-width=\"1\" opacity=\"0.3\"/>\n";
                svg << "    </pattern>\n";
                svg << "  </defs>\n";
                svg << "  <rect width=\"100%\" height=\"100%\" fill=\"url(#grid)\"/>\n";
            }

            // Plot lines
            for (const auto &s : series)
            {
                if (s.points.empty() || !s.visible)
                    continue;

                std::string color = s.color.empty() ? "#00ff00" : s.color;

                // Create path
                std::ostringstream path;
                for (size_t i = 0; i < s.points.size(); ++i)
                {
                    const auto &point = s.points[i];
                    const double x = margin + (point.x - min_x + x_padding) / (x_range + 2 * x_padding) * chart_width;
                    const double y = margin + (max_y + y_padding - point.y) / (y_range + 2 * y_padding) * chart_height;

                    if (i == 0)
                    {
                        path << "M " << x << " " << y;
                    }
                    else
                    {
                        path << " L " << x << " " << y;
                    }
                }

                svg << "  <path d=\"" << path.str() << "\" fill=\"none\" stroke=\"" << color << "\" stroke-width=\"2\"/>\n";
            }

            // Title
            svg << "  <text x=\"" << current_config_.width / 2 << "\" y=\"25\" ";
            svg << "text-anchor=\"middle\" fill=\"" << current_config_.text_color << "\" font-size=\"16\" font-weight=\"bold\">";
            svg << current_config_.title << "</text>\n";

            // Legend
            if (current_config_.show_legend)
            {
                int legend_y = 40;
                for (const auto &s : series)
                {
                    if (!s.visible)
                        continue;
                    std::string color = s.color.empty() ? "#00ff00" : s.color;
                    svg << "  <rect x=\"10\" y=\"" << legend_y - 10 << "\" width=\"15\" height=\"10\" fill=\"" << color << "\"/>\n";
                    svg << "  <text x=\"30\" y=\"" << legend_y << "\" fill=\"" << current_config_.text_color << "\" font-size=\"12\">" << s.name << "</text>\n";
                    legend_y += 20;
                }
            }

            svg << "</svg>";
            return svg.str();
        }

        std::string HTMLChartRenderer::generate_svg_bar(const std::vector<ChartSeries> &series)
        {
            if (series.empty() || series[0].points.empty())
                return "";

            std::ostringstream svg;

            // Calculate dimensions
            const int margin = 50;
            const int chart_width = current_config_.width - 2 * margin;
            const int chart_height = current_config_.height - 2 * margin;

            // Find data range
            double min_y = 0;
            double max_y = series[0].points[0].y;

            for (const auto &s : series)
            {
                for (const auto &point : s.points)
                {
                    max_y = std::max(max_y, point.y);
                }
            }

            const double y_range = max_y - min_y;
            const double y_padding = y_range * 0.1;

            // SVG header
            svg << "<svg width=\"" << current_config_.width << "\" height=\"" << current_config_.height << "\" ";
            svg << "style=\"background-color: " << current_config_.background_color << ";\">\n";

            // Grid
            if (current_config_.show_grid)
            {
                svg << "  <defs>\n";
                svg << "    <pattern id=\"grid\" width=\"40\" height=\"40\" patternUnits=\"userSpaceOnUse\">\n";
                svg << "      <path d=\"M 40 0 L 0 0 0 40\" fill=\"none\" stroke=\"" << current_config_.grid_color << "\" stroke-width=\"1\" opacity=\"0.3\"/>\n";
                svg << "    </pattern>\n";
                svg << "  </defs>\n";
                svg << "  <rect width=\"100%\" height=\"100%\" fill=\"url(#grid)\"/>\n";
            }

            // Plot bars
            for (const auto &s : series)
            {
                if (s.points.empty() || !s.visible)
                    continue;

                std::string color = s.color.empty() ? "#0088ff" : s.color;
                const double bar_width = static_cast<double>(chart_width) / s.points.size() * 0.8;
                const double spacing = static_cast<double>(chart_width) / s.points.size();

                for (size_t i = 0; i < s.points.size(); ++i)
                {
                    const auto &point = s.points[i];
                    const double x = margin + i * spacing + spacing / 2 - bar_width / 2;
                    const double y = margin + (max_y + y_padding - point.y) / (y_range + 2 * y_padding) * chart_height;
                    const double height = point.y / (y_range + 2 * y_padding) * chart_height;

                    svg << "  <rect x=\"" << x << "\" y=\"" << y << "\" ";
                    svg << "width=\"" << bar_width << "\" height=\"" << height << "\" ";
                    svg << "fill=\"" << color << "\" stroke=\"" << get_color_scheme(color) << "\" stroke-width=\"1\"/>\n";
                }
            }

            // Title
            svg << "  <text x=\"" << current_config_.width / 2 << "\" y=\"25\" ";
            svg << "text-anchor=\"middle\" fill=\"" << current_config_.text_color << "\" font-size=\"16\" font-weight=\"bold\">";
            svg << current_config_.title << "</text>\n";

            svg << "</svg>";
            return svg.str();
        }

        std::string HTMLChartRenderer::generate_svg_indicators(const std::vector<IndicatorOverlay> &indicators)
        {
            if (indicators.empty())
                return "";

            std::ostringstream svg;

            // Calculate dimensions
            const int margin = 50;
            const int chart_width = current_config_.width - 2 * margin;
            const int chart_height = current_config_.height - 2 * margin;

            // Find data range
            double min_y = indicators[0].points[0].y;
            double max_y = indicators[0].points[0].y;

            for (const auto &indicator : indicators)
            {
                for (const auto &point : indicator.points)
                {
                    min_y = std::min(min_y, point.y);
                    max_y = std::max(max_y, point.y);
                }
            }

            const double y_range = max_y - min_y;
            const double y_padding = y_range * 0.1;

            // SVG header
            svg << "<svg width=\"" << current_config_.width << "\" height=\"" << current_config_.height << "\" ";
            svg << "style=\"background-color: " << current_config_.background_color << ";\">\n";

            // Plot indicators
            for (const auto &indicator : indicators)
            {
                if (indicator.points.empty() || !indicator.visible)
                    continue;

                std::string color = indicator.color.empty() ? "#ffff00" : indicator.color;

                // Create path
                std::ostringstream path;
                for (size_t i = 0; i < indicator.points.size(); ++i)
                {
                    const auto &point = indicator.points[i];
                    const double x = margin + (static_cast<double>(i) / (indicator.points.size() - 1)) * chart_width;
                    const double y = margin + (max_y + y_padding - point.y) / (y_range + 2 * y_padding) * chart_height;

                    if (i == 0)
                    {
                        path << "M " << x << " " << y;
                    }
                    else
                    {
                        path << " L " << x << " " << y;
                    }
                }

                svg << "  <path d=\"" << path.str() << "\" fill=\"none\" stroke=\"" << color << "\" ";
                svg << "stroke-width=\"2\" opacity=\"" << indicator.opacity << "\"/>\n";
            }

            // Title
            svg << "  <text x=\"" << current_config_.width / 2 << "\" y=\"25\" ";
            svg << "text-anchor=\"middle\" fill=\"" << current_config_.text_color << "\" font-size=\"16\" font-weight=\"bold\">";
            svg << current_config_.title << "</text>\n";

            svg << "</svg>";
            return svg.str();
        }

        std::string HTMLChartRenderer::generate_html_wrapper(const std::string &svg_content)
        {
            std::ostringstream html;
            html << "<!DOCTYPE html>\n";
            html << "<html>\n";
            html << "<head>\n";
            html << "  <title>" << current_config_.title << "</title>\n";
            html << "  <style>\n";
            html << "    body { margin: 0; padding: 20px; font-family: Arial, sans-serif; }\n";
            html << "    .chart-container { text-align: center; }\n";
            html << "  </style>\n";
            html << "</head>\n";
            html << "<body>\n";
            html << "  <div class=\"chart-container\">\n";
            html << svg_content << "\n";
            html << "  </div>\n";
            html << "</body>\n";
            html << "</html>";
            return html.str();
        }

        std::string HTMLChartRenderer::generate_json_data()
        {
            std::ostringstream json;
            json << "{\n";
            json << "  \"title\": \"" << current_config_.title << "\",\n";
            json << "  \"width\": " << current_config_.width << ",\n";
            json << "  \"height\": " << current_config_.height << ",\n";
            json << "  \"series\": [\n";

            for (size_t i = 0; i < current_series_.size(); ++i)
            {
                const auto &series = current_series_[i];
                json << "    {\n";
                json << "      \"name\": \"" << series.name << "\",\n";
                json << "      \"type\": " << static_cast<int>(series.type) << ",\n";
                json << "      \"color\": \"" << series.color << "\",\n";
                json << "      \"visible\": " << (series.visible ? "true" : "false") << ",\n";
                json << "      \"points\": [\n";

                for (size_t j = 0; j < series.points.size(); ++j)
                {
                    const auto &point = series.points[j];
                    json << "        {\"x\": " << point.x << ", \"y\": " << point.y;
                    if (!point.label.empty())
                    {
                        json << ", \"label\": \"" << point.label << "\"";
                    }
                    if (!point.color.empty())
                    {
                        json << ", \"color\": \"" << point.color << "\"";
                    }
                    json << "}";
                    if (j < series.points.size() - 1)
                        json << ",";
                    json << "\n";
                }

                json << "      ]\n";
                json << "    }";
                if (i < current_series_.size() - 1)
                    json << ",";
                json << "\n";
            }

            json << "  ]\n";
            json << "}";
            return json.str();
        }

        std::string HTMLChartRenderer::format_timestamp(const std::chrono::system_clock::time_point &timestamp)
        {
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        std::string HTMLChartRenderer::format_number(double value, int precision)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }

        std::string HTMLChartRenderer::get_color_scheme(const std::string &base_color)
        {
            // Simple color darkening for borders
            if (base_color == "#00ff00")
                return "#00cc00";
            if (base_color == "#ff0000")
                return "#cc0000";
            if (base_color == "#0088ff")
                return "#0066cc";
            return base_color;
        }

        // ConsoleChartRenderer implementation
        bool ConsoleChartRenderer::initialize(const ChartConfig &config)
        {
            current_config_ = config;
            return true;
        }

        bool ConsoleChartRenderer::render_candlestick_chart(
            const std::vector<CandlestickPoint> &data,
            const std::vector<IndicatorOverlay> &indicators,
            const ChartConfig &config)
        {

            if (config.width > 0)
                current_config_.width = config.width;
            if (config.height > 0)
                current_config_.height = config.height;

            std::cout << "\n=== " << current_config_.title << " ===\n\n";
            render_candlestick_console(data);

            if (!indicators.empty())
            {
                render_indicators_console(indicators);
            }

            if (current_config_.show_volume)
            {
                render_volume_console(data);
            }

            return true;
        }

        bool ConsoleChartRenderer::render_line_chart(
            const std::vector<ChartSeries> &series,
            const ChartConfig &config)
        {

            if (config.width > 0)
                current_config_.width = config.width;
            if (config.height > 0)
                current_config_.height = config.height;

            current_series_ = series;

            std::cout << "\n=== " << current_config_.title << " ===\n\n";
            render_line_console(series);

            return true;
        }

        bool ConsoleChartRenderer::render_bar_chart(
            const std::vector<ChartSeries> &series,
            const ChartConfig &config)
        {

            if (config.width > 0)
                current_config_.width = config.width;
            if (config.height > 0)
                current_config_.height = config.height;

            current_series_ = series;

            std::cout << "\n=== " << current_config_.title << " ===\n\n";

            // For console, bar charts are similar to line charts
            render_line_console(series);

            return true;
        }

        bool ConsoleChartRenderer::render_indicators(
            const std::vector<IndicatorOverlay> &indicators,
            const ChartConfig &config)
        {

            if (config.width > 0)
                current_config_.width = config.width;
            if (config.height > 0)
                current_config_.height = config.height;

            std::cout << "\n=== " << current_config_.title << " ===\n\n";
            render_indicators_console(indicators);

            return true;
        }

        bool ConsoleChartRenderer::export_to_file(const std::string &filename, const std::string &format)
        {
            // For console renderer, we'll save a simple text representation
            std::ofstream file(filename);
            if (!file.is_open())
            {
                return false;
            }

            file << "Chart: " << current_config_.title << "\n";
            file << "Generated: " << format_timestamp(std::chrono::system_clock::now()) << "\n\n";

            // Add some basic chart data
            for (const auto &series : current_series_)
            {
                file << "Series: " << series.name << "\n";
                for (const auto &point : series.points)
                {
                    file << "  " << point.x << ": " << point.y << "\n";
                }
                file << "\n";
            }

            file.close();
            return true;
        }

        std::string ConsoleChartRenderer::get_chart_data(const std::string &format)
        {
            std::ostringstream data;
            data << "Chart: " << current_config_.title << "\n";
            data << "Format: " << format << "\n";
            data << "Series count: " << current_series_.size() << "\n";

            for (const auto &series : current_series_)
            {
                data << "Series: " << series.name << " (" << series.points.size() << " points)\n";
            }

            return data.str();
        }

        void ConsoleChartRenderer::clear()
        {
            current_series_.clear();
        }

        // Helper methods for ConsoleChartRenderer
        void ConsoleChartRenderer::render_candlestick_console(const std::vector<CandlestickPoint> &data)
        {
            if (data.empty())
            {
                std::cout << "No data to display\n";
                return;
            }

            // Find price range
            double min_price = data[0].low;
            double max_price = data[0].high;
            for (const auto &point : data)
            {
                min_price = std::min(min_price, point.low);
                max_price = std::max(max_price, point.high);
            }

            const double price_range = max_price - min_price;
            const int chart_height = 20;

            std::cout << "Price Range: $" << format_price(min_price) << " - $" << format_price(max_price) << "\n\n";

            // Display candlesticks
            for (size_t i = 0; i < data.size() && i < 50; ++i)
            { // Limit to 50 for console
                const auto &point = data[i];
                const double body_ratio = std::abs(point.close - point.open) / price_range;

                std::cout << std::setw(3) << i << ": ";
                std::cout << "O:" << format_price(point.open) << " ";
                std::cout << "H:" << format_price(point.high) << " ";
                std::cout << "L:" << format_price(point.low) << " ";
                std::cout << "C:" << format_price(point.close) << " ";
                std::cout << "V:" << format_volume(point.volume) << " ";
                std::cout << get_candlestick_symbol(point.is_green, body_ratio) << "\n";
            }
        }

        void ConsoleChartRenderer::render_line_console(const std::vector<ChartSeries> &series)
        {
            for (const auto &s : series)
            {
                if (s.points.empty() || !s.visible)
                    continue;

                std::cout << "Series: " << s.name << "\n";

                // Find range
                double min_val = s.points[0].y;
                double max_val = s.points[0].y;
                for (const auto &point : s.points)
                {
                    min_val = std::min(min_val, point.y);
                    max_val = std::max(max_val, point.y);
                }

                const double range = max_val - min_val;
                const int chart_height = 15;

                // Simple ASCII chart
                for (int row = chart_height; row >= 0; --row)
                {
                    std::cout << std::setw(3) << (min_val + (row * range / chart_height)) << " |";

                    for (size_t i = 0; i < s.points.size() && i < 60; ++i)
                    { // Limit width
                        const auto &point = s.points[i];
                        const int y_pos = static_cast<int>((point.y - min_val) / range * chart_height);

                        if (y_pos == row)
                        {
                            std::cout << "*";
                        }
                        else
                        {
                            std::cout << " ";
                        }
                    }
                    std::cout << "\n";
                }

                std::cout << "     +";
                for (int i = 0; i < 60; ++i)
                    std::cout << "-";
                std::cout << "\n\n";
            }
        }

        void ConsoleChartRenderer::render_volume_console(const std::vector<CandlestickPoint> &data)
        {
            if (data.empty())
                return;

            std::cout << "Volume Profile:\n";

            // Find max volume
            uint64_t max_volume = data[0].volume;
            for (const auto &point : data)
            {
                max_volume = std::max(max_volume, point.volume);
            }

            // Display volume bars
            for (size_t i = 0; i < data.size() && i < 30; ++i)
            { // Limit to 30 for console
                const auto &point = data[i];
                const int bar_length = static_cast<int>((static_cast<double>(point.volume) / max_volume) * 20);

                std::cout << std::setw(3) << i << ": ";
                std::cout << format_volume(point.volume) << " ";
                for (int j = 0; j < bar_length; ++j)
                {
                    std::cout << "#";
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        void ConsoleChartRenderer::render_indicators_console(const std::vector<IndicatorOverlay> &indicators)
        {
            for (const auto &indicator : indicators)
            {
                if (indicator.points.empty() || !indicator.visible)
                    continue;

                std::cout << "Indicator: " << indicator.name << "\n";

                // Find range
                double min_val = indicator.points[0].y;
                double max_val = indicator.points[0].y;
                for (const auto &point : indicator.points)
                {
                    min_val = std::min(min_val, point.y);
                    max_val = std::max(max_val, point.y);
                }

                const double range = max_val - min_val;
                const int chart_height = 10;

                // Simple ASCII chart
                for (int row = chart_height; row >= 0; --row)
                {
                    std::cout << std::setw(6) << (min_val + (row * range / chart_height)) << " |";

                    for (size_t i = 0; i < indicator.points.size() && i < 40; ++i)
                    {
                        const auto &point = indicator.points[i];
                        const int y_pos = static_cast<int>((point.y - min_val) / range * chart_height);

                        if (y_pos == row)
                        {
                            std::cout << "+";
                        }
                        else
                        {
                            std::cout << " ";
                        }
                    }
                    std::cout << "\n";
                }

                std::cout << "       +";
                for (int i = 0; i < 40; ++i)
                    std::cout << "-";
                std::cout << "\n\n";
            }
        }

        std::string ConsoleChartRenderer::format_price(double price, int width)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << std::setw(width) << price;
            return oss.str();
        }

        std::string ConsoleChartRenderer::format_volume(uint64_t volume)
        {
            if (volume >= 1000000000)
            {
                return std::to_string(volume / 1000000000) + "B";
            }
            else if (volume >= 1000000)
            {
                return std::to_string(volume / 1000000) + "M";
            }
            else if (volume >= 1000)
            {
                return std::to_string(volume / 1000) + "K";
            }
            else
            {
                return std::to_string(volume);
            }
        }

        std::string ConsoleChartRenderer::format_timestamp(const std::chrono::system_clock::time_point &timestamp)
        {
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }

        std::string ConsoleChartRenderer::get_candlestick_symbol(bool is_green, double body_ratio)
        {
            if (is_green)
            {
                if (body_ratio > 0.1)
                    return "█";
                else
                    return "│";
            }
            else
            {
                if (body_ratio > 0.1)
                    return "▓";
                else
                    return "│";
            }
        }

    } // namespace visualization
} // namespace trading
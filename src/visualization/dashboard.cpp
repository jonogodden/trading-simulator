#include "visualization/dashboard.h"
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

        // DashboardWidget implementation
        void DashboardWidget::mark_for_update()
        {
            needs_update_ = true;
        }

        void DashboardWidget::clear_update_flag()
        {
            needs_update_ = false;
        }

        // PriceTickerWidget implementation
        void PriceTickerWidget::update()
        {
            if (needs_update_)
            {
                // Update logic would go here
                clear_update_flag();
            }
        }

        void PriceTickerWidget::render()
        {
            std::cout << "=== " << config_.title << " ===\n";
            std::cout << "Symbol: " << symbol_ << "\n";
            std::cout << "Price: $" << std::fixed << std::setprecision(2) << current_price_ << "\n";
            std::cout << "Change: " << (price_change_ >= 0 ? "+" : "") << std::fixed << std::setprecision(2) << price_change_ << " (" << price_change_percent_ << "%)\n";
            std::cout << "Volume: " << format_volume(volume_) << "\n";
            std::cout << "Last Update: " << format_timestamp(last_update_) << "\n\n";
        }

        std::string PriceTickerWidget::get_data() const
        {
            std::ostringstream data;
            data << "{\n";
            data << "  \"symbol\": \"" << symbol_ << "\",\n";
            data << "  \"price\": " << current_price_ << ",\n";
            data << "  \"change\": " << price_change_ << ",\n";
            data << "  \"change_percent\": " << price_change_percent_ << ",\n";
            data << "  \"volume\": " << volume_ << ",\n";
            data << "  \"timestamp\": \"" << format_timestamp(last_update_) << "\"\n";
            data << "}";
            return data.str();
        }

        void PriceTickerWidget::set_data(const std::string &data)
        {
            // Simple JSON parsing for demo purposes
            // In a real implementation, you'd use a proper JSON parser
            if (data.find("\"price\"") != std::string::npos)
            {
                // Extract price from JSON-like string
                size_t pos = data.find("\"price\"");
                if (pos != std::string::npos)
                {
                    pos = data.find(":", pos);
                    if (pos != std::string::npos)
                    {
                        pos = data.find_first_not_of(" \t", pos + 1);
                        if (pos != std::string::npos)
                        {
                            size_t end = data.find_first_of(",}", pos);
                            if (end != std::string::npos)
                            {
                                std::string price_str = data.substr(pos, end - pos);
                                try
                                {
                                    current_price_ = std::stod(price_str);
                                }
                                catch (...)
                                {
                                    // Ignore parsing errors
                                }
                            }
                        }
                    }
                }
            }
        }

        void PriceTickerWidget::update_price(double price, double change, double change_percent, uint64_t volume)
        {
            current_price_ = price;
            price_change_ = change;
            price_change_percent_ = change_percent;
            volume_ = volume;
            last_update_ = std::chrono::system_clock::now();
            mark_for_update();
        }

        std::string PriceTickerWidget::format_timestamp(const std::chrono::system_clock::time_point &timestamp) const
        {
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
            return oss.str();
        }

        std::string PriceTickerWidget::format_volume(uint64_t volume) const
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

        // ChartWidget implementation
        ChartWidget::ChartWidget(const std::string &widget_id, const WidgetConfig &cfg,
                                 std::unique_ptr<ChartRenderer> renderer)
            : DashboardWidget(widget_id, cfg), renderer_(std::move(renderer))
        {
            chart_config_.width = cfg.width;
            chart_config_.height = cfg.height;
            chart_config_.title = cfg.title;
        }

        void ChartWidget::update()
        {
            if (needs_update_ && renderer_)
            {
                if (!candlestick_data_.empty())
                {
                    renderer_->render_candlestick_chart(candlestick_data_, indicators_, chart_config_);
                }
                else if (!current_series_.empty())
                {
                    renderer_->render_line_chart(current_series_, chart_config_);
                }
                clear_update_flag();
            }
        }

        void ChartWidget::render()
        {
            std::cout << "=== " << config_.title << " ===\n";
            if (renderer_)
            {
                std::cout << "Chart rendered with " << candlestick_data_.size() << " candlesticks\n";
                std::cout << "Indicators: " << indicators_.size() << "\n";
                std::cout << "Chart size: " << chart_config_.width << "x" << chart_config_.height << "\n\n";
            }
            else
            {
                std::cout << "No renderer available\n\n";
            }
        }

        std::string ChartWidget::get_data() const
        {
            if (renderer_)
            {
                return renderer_->get_chart_data("json");
            }
            return "{}";
        }

        void ChartWidget::set_data(const std::string &data)
        {
            // Parse chart data from JSON string
            // This is a simplified implementation
            if (data.find("candlestick") != std::string::npos)
            {
                // Would parse candlestick data
            }
            else if (data.find("series") != std::string::npos)
            {
                // Would parse series data
            }
        }

        void ChartWidget::update_candlestick_data(const std::vector<CandlestickPoint> &data)
        {
            candlestick_data_ = data;
            mark_for_update();
        }

        void ChartWidget::update_indicators(const std::vector<IndicatorOverlay> &indicators)
        {
            indicators_ = indicators;
            mark_for_update();
        }

        void ChartWidget::set_chart_config(const ChartConfig &config)
        {
            chart_config_ = config;
            mark_for_update();
        }

        bool ChartWidget::export_chart(const std::string &filename)
        {
            if (renderer_)
            {
                return renderer_->export_to_file(filename);
            }
            return false;
        }

        // OrderBookWidget implementation
        void OrderBookWidget::update()
        {
            if (needs_update_)
            {
                // Calculate spread and totals
                if (!bids_.empty() && !asks_.empty())
                {
                    spread_ = asks_[0].price - bids_[0].price;
                }

                total_bid_volume_ = 0;
                total_ask_volume_ = 0;

                for (const auto &bid : bids_)
                {
                    total_bid_volume_ += bid.quantity;
                }
                for (const auto &ask : asks_)
                {
                    total_ask_volume_ += ask.quantity;
                }

                clear_update_flag();
            }
        }

        void OrderBookWidget::render()
        {
            std::cout << "=== " << config_.title << " ===\n";
            std::cout << "Spread: $" << std::fixed << std::setprecision(2) << spread_ << "\n";
            std::cout << "Total Bid Volume: " << format_volume(total_bid_volume_) << "\n";
            std::cout << "Total Ask Volume: " << format_volume(total_ask_volume_) << "\n\n";

            std::cout << "Asks (Sell Orders):\n";
            for (size_t i = 0; i < std::min(asks_.size(), size_t(5)); ++i)
            {
                const auto &ask = asks_[i];
                std::cout << "  $" << std::fixed << std::setprecision(2) << ask.price
                          << " - " << format_volume(ask.quantity)
                          << " (" << ask.order_count << " orders)\n";
            }

            std::cout << "\nBids (Buy Orders):\n";
            for (size_t i = 0; i < std::min(bids_.size(), size_t(5)); ++i)
            {
                const auto &bid = bids_[i];
                std::cout << "  $" << std::fixed << std::setprecision(2) << bid.price
                          << " - " << format_volume(bid.quantity)
                          << " (" << bid.order_count << " orders)\n";
            }
            std::cout << "\n";
        }

        std::string OrderBookWidget::get_data() const
        {
            std::ostringstream data;
            data << "{\n";
            data << "  \"spread\": " << spread_ << ",\n";
            data << "  \"total_bid_volume\": " << total_bid_volume_ << ",\n";
            data << "  \"total_ask_volume\": " << total_ask_volume_ << ",\n";
            data << "  \"asks\": [\n";
            for (size_t i = 0; i < asks_.size(); ++i)
            {
                const auto &ask = asks_[i];
                data << "    {\"price\": " << ask.price << ", \"quantity\": " << ask.quantity << ", \"orders\": " << ask.order_count << "}";
                if (i < asks_.size() - 1)
                    data << ",";
                data << "\n";
            }
            data << "  ],\n";
            data << "  \"bids\": [\n";
            for (size_t i = 0; i < bids_.size(); ++i)
            {
                const auto &bid = bids_[i];
                data << "    {\"price\": " << bid.price << ", \"quantity\": " << bid.quantity << ", \"orders\": " << bid.order_count << "}";
                if (i < bids_.size() - 1)
                    data << ",";
                data << "\n";
            }
            data << "  ]\n";
            data << "}";
            return data.str();
        }

        void OrderBookWidget::set_data(const std::string &data)
        {
            // Parse order book data from JSON string
            // This is a simplified implementation
            if (data.find("asks") != std::string::npos && data.find("bids") != std::string::npos)
            {
                // Would parse order book data
                mark_for_update();
            }
        }

        void OrderBookWidget::update_order_book(const std::vector<OrderBookLevel> &bids,
                                                const std::vector<OrderBookLevel> &asks)
        {
            bids_ = bids;
            asks_ = asks;
            mark_for_update();
        }

        std::string OrderBookWidget::format_volume(uint64_t volume)
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

        // PositionSummaryWidget implementation
        void PositionSummaryWidget::update()
        {
            if (needs_update_)
            {
                // Calculate P&L for each position
                for (auto &position : positions_)
                {
                    position.unrealized_pnl = (position.current_price - position.avg_price) * position.quantity;
                }

                // Calculate totals
                total_unrealized_pnl_ = 0;
                total_realized_pnl_ = 0;

                for (const auto &position : positions_)
                {
                    total_unrealized_pnl_ += position.unrealized_pnl;
                    total_realized_pnl_ += position.realized_pnl;
                }

                clear_update_flag();
            }
        }

        void PositionSummaryWidget::render()
        {
            std::cout << "=== " << config_.title << " ===\n";
            std::cout << "Portfolio Value: $" << std::fixed << std::setprecision(2) << total_portfolio_value_ << "\n";
            std::cout << "Total P&L: $" << std::fixed << std::setprecision(2) << total_pnl() << "\n";
            std::cout << "  Unrealized: $" << std::fixed << std::setprecision(2) << total_unrealized_pnl_ << "\n";
            std::cout << "  Realized: $" << std::fixed << std::setprecision(2) << total_realized_pnl_ << "\n\n";

            std::cout << "Positions:\n";
            for (const auto &position : positions_)
            {
                std::cout << "  " << position.symbol << ": " << position.quantity << " @ $"
                          << std::fixed << std::setprecision(2) << position.avg_price
                          << " (Current: $" << position.current_price << ")\n";
                std::cout << "    P&L: $" << std::fixed << std::setprecision(2) << position.unrealized_pnl << "\n";
            }
            std::cout << "\n";
        }

        std::string PositionSummaryWidget::get_data() const
        {
            std::ostringstream data;
            data << "{\n";
            data << "  \"portfolio_value\": " << total_portfolio_value_ << ",\n";
            data << "  \"total_unrealized_pnl\": " << total_unrealized_pnl_ << ",\n";
            data << "  \"total_realized_pnl\": " << total_realized_pnl_ << ",\n";
            data << "  \"positions\": [\n";
            for (size_t i = 0; i < positions_.size(); ++i)
            {
                const auto &position = positions_[i];
                data << "    {\n";
                data << "      \"symbol\": \"" << position.symbol << "\",\n";
                data << "      \"quantity\": " << position.quantity << ",\n";
                data << "      \"avg_price\": " << position.avg_price << ",\n";
                data << "      \"current_price\": " << position.current_price << ",\n";
                data << "      \"unrealized_pnl\": " << position.unrealized_pnl << ",\n";
                data << "      \"realized_pnl\": " << position.realized_pnl << "\n";
                data << "    }";
                if (i < positions_.size() - 1)
                    data << ",";
                data << "\n";
            }
            data << "  ]\n";
            data << "}";
            return data.str();
        }

        void PositionSummaryWidget::set_data(const std::string &data)
        {
            // Parse position data from JSON string
            // This is a simplified implementation
            if (data.find("positions") != std::string::npos)
            {
                // Would parse position data
                mark_for_update();
            }
        }

        void PositionSummaryWidget::update_positions(const std::vector<Position> &positions)
        {
            positions_ = positions;
            mark_for_update();
        }

        void PositionSummaryWidget::update_portfolio_value(double value)
        {
            total_portfolio_value_ = value;
            mark_for_update();
        }

        // PnLChartWidget implementation
        PnLChartWidget::PnLChartWidget(const std::string &widget_id, const WidgetConfig &cfg,
                                       std::unique_ptr<ChartRenderer> renderer)
            : DashboardWidget(widget_id, cfg), renderer_(std::move(renderer)),
              max_drawdown_(0.0), total_return_(0.0)
        {
            chart_config_.width = cfg.width;
            chart_config_.height = cfg.height;
            chart_config_.title = cfg.title;
        }

        void PnLChartWidget::update()
        {
            if (needs_update_ && renderer_)
            {
                // Create chart series from P&L data
                std::vector<ChartSeries> series;

                if (!pnl_data_.empty())
                {
                    ChartSeries pnl_series("P&L", ChartType::LINE, "#00ff00");
                    for (const auto &point : pnl_data_)
                    {
                        pnl_series.points.push_back(point);
                    }
                    series.push_back(std::move(pnl_series));
                }

                if (!drawdown_data_.empty())
                {
                    ChartSeries dd_series("Drawdown", ChartType::LINE, "#ff0000");
                    for (const auto &point : drawdown_data_)
                    {
                        dd_series.points.push_back(point);
                    }
                    series.push_back(std::move(dd_series));
                }

                if (!series.empty())
                {
                    renderer_->render_line_chart(series, chart_config_);
                }

                clear_update_flag();
            }
        }

        void PnLChartWidget::render()
        {
            std::cout << "=== " << config_.title << " ===\n";
            std::cout << "Total Return: " << std::fixed << std::setprecision(2) << (total_return_ * 100) << "%\n";
            std::cout << "Max Drawdown: " << std::fixed << std::setprecision(2) << (max_drawdown_ * 100) << "%\n";
            std::cout << "P&L Data Points: " << pnl_data_.size() << "\n";
            std::cout << "Drawdown Data Points: " << drawdown_data_.size() << "\n\n";
        }

        std::string PnLChartWidget::get_data() const
        {
            std::ostringstream data;
            data << "{\n";
            data << "  \"total_return\": " << total_return_ << ",\n";
            data << "  \"max_drawdown\": " << max_drawdown_ << ",\n";
            data << "  \"pnl_points\": " << pnl_data_.size() << ",\n";
            data << "  \"drawdown_points\": " << drawdown_data_.size() << "\n";
            data << "}";
            return data.str();
        }

        void PnLChartWidget::set_data(const std::string &data)
        {
            // Parse P&L data from JSON string
            // This is a simplified implementation
            if (data.find("pnl") != std::string::npos)
            {
                // Would parse P&L data
                mark_for_update();
            }
        }

        void PnLChartWidget::update_pnl_data(const std::vector<ChartPoint> &pnl, const std::vector<ChartPoint> &drawdown)
        {
            pnl_data_ = pnl;
            drawdown_data_ = drawdown;
            mark_for_update();
        }

        void PnLChartWidget::update_metrics(double max_dd, double total_ret)
        {
            max_drawdown_ = max_dd;
            total_return_ = total_ret;
            mark_for_update();
        }

        // PerformanceMetricsWidget implementation
        void PerformanceMetricsWidget::update()
        {
            if (needs_update_)
            {
                // Update logic would go here
                clear_update_flag();
            }
        }

        void PerformanceMetricsWidget::render()
        {
            std::cout << "=== " << config_.title << " ===\n";
            std::cout << "Sharpe Ratio: " << std::fixed << std::setprecision(3) << metrics_.sharpe_ratio << "\n";
            std::cout << "Sortino Ratio: " << std::fixed << std::setprecision(3) << metrics_.sortino_ratio << "\n";
            std::cout << "Calmar Ratio: " << std::fixed << std::setprecision(3) << metrics_.calmar_ratio << "\n";
            std::cout << "Max Drawdown: " << std::fixed << std::setprecision(2) << (metrics_.max_drawdown * 100) << "%\n";
            std::cout << "Total Return: " << std::fixed << std::setprecision(2) << (metrics_.total_return * 100) << "%\n";
            std::cout << "Annualized Return: " << std::fixed << std::setprecision(2) << (metrics_.annualized_return * 100) << "%\n";
            std::cout << "Volatility: " << std::fixed << std::setprecision(2) << (metrics_.volatility * 100) << "%\n";
            std::cout << "Win Rate: " << std::fixed << std::setprecision(2) << (metrics_.win_rate * 100) << "%\n";
            std::cout << "Total Trades: " << metrics_.total_trades << "\n";
            std::cout << "Avg Win: $" << std::fixed << std::setprecision(2) << metrics_.avg_win << "\n";
            std::cout << "Avg Loss: $" << std::fixed << std::setprecision(2) << metrics_.avg_loss << "\n";
            std::cout << "Profit Factor: " << std::fixed << std::setprecision(3) << metrics_.profit_factor << "\n\n";
        }

        std::string PerformanceMetricsWidget::get_data() const
        {
            std::ostringstream data;
            data << "{\n";
            data << "  \"sharpe_ratio\": " << metrics_.sharpe_ratio << ",\n";
            data << "  \"sortino_ratio\": " << metrics_.sortino_ratio << ",\n";
            data << "  \"calmar_ratio\": " << metrics_.calmar_ratio << ",\n";
            data << "  \"max_drawdown\": " << metrics_.max_drawdown << ",\n";
            data << "  \"total_return\": " << metrics_.total_return << ",\n";
            data << "  \"annualized_return\": " << metrics_.annualized_return << ",\n";
            data << "  \"volatility\": " << metrics_.volatility << ",\n";
            data << "  \"win_rate\": " << metrics_.win_rate << ",\n";
            data << "  \"total_trades\": " << metrics_.total_trades << ",\n";
            data << "  \"avg_win\": " << metrics_.avg_win << ",\n";
            data << "  \"avg_loss\": " << metrics_.avg_loss << ",\n";
            data << "  \"profit_factor\": " << metrics_.profit_factor << "\n";
            data << "}";
            return data.str();
        }

        void PerformanceMetricsWidget::set_data(const std::string &data)
        {
            // Parse metrics data from JSON string
            // This is a simplified implementation
            if (data.find("sharpe_ratio") != std::string::npos)
            {
                // Would parse metrics data
                mark_for_update();
            }
        }

        void PerformanceMetricsWidget::update_metrics(const Metrics &metrics)
        {
            metrics_ = metrics;
            mark_for_update();
        }

        // DashboardPanel implementation
        void DashboardPanel::add_widget(std::unique_ptr<DashboardWidget> widget)
        {
            widgets_.push_back(std::move(widget));
        }

        void DashboardPanel::remove_widget(const std::string &widget_id)
        {
            widgets_.erase(
                std::remove_if(widgets_.begin(), widgets_.end(),
                               [&widget_id](const std::unique_ptr<DashboardWidget> &widget)
                               {
                                   return widget->id() == widget_id;
                               }),
                widgets_.end());
        }

        DashboardWidget *DashboardPanel::get_widget(const std::string &widget_id)
        {
            for (auto &widget : widgets_)
            {
                if (widget->id() == widget_id)
                {
                    return widget.get();
                }
            }
            return nullptr;
        }

        void DashboardPanel::render()
        {
            if (!visible_)
                return;

            std::cout << "=== Panel: " << title_ << " ===\n";
            std::cout << "Position: (" << x_ << ", " << y_ << "), Size: " << width_ << "x" << height_ << "\n\n";

            for (auto &widget : widgets_)
            {
                if (widget->config().visible)
                {
                    widget->render();
                }
            }
        }

        void DashboardPanel::update()
        {
            for (auto &widget : widgets_)
            {
                widget->update();
            }
        }

        // Dashboard implementation
        void Dashboard::add_panel(std::unique_ptr<DashboardPanel> panel)
        {
            panels_.push_back(std::move(panel));
        }

        void Dashboard::remove_panel(const std::string &panel_id)
        {
            panels_.erase(
                std::remove_if(panels_.begin(), panels_.end(),
                               [&panel_id](const std::unique_ptr<DashboardPanel> &panel)
                               {
                                   return panel->id() == panel_id;
                               }),
                panels_.end());
        }

        DashboardPanel *Dashboard::get_panel(const std::string &panel_id)
        {
            for (auto &panel : panels_)
            {
                if (panel->id() == panel_id)
                {
                    return panel.get();
                }
            }
            return nullptr;
        }

        void Dashboard::add_widget_to_panel(const std::string &panel_id, std::unique_ptr<DashboardWidget> widget)
        {
            auto panel = get_panel(panel_id);
            if (panel)
            {
                panel->add_widget(std::move(widget));
            }
        }

        DashboardWidget *Dashboard::get_widget(const std::string &widget_id)
        {
            for (auto &panel : panels_)
            {
                auto widget = panel->get_widget(widget_id);
                if (widget)
                {
                    return widget;
                }
            }
            return nullptr;
        }

        void Dashboard::register_data_source(const std::string &source_id, DataUpdateCallback callback)
        {
            data_sources_[source_id] = callback;
        }

        void Dashboard::unregister_data_source(const std::string &source_id)
        {
            data_sources_.erase(source_id);
        }

        void Dashboard::update_data_source(const std::string &source_id, const std::string &data)
        {
            auto it = data_sources_.find(source_id);
            if (it != data_sources_.end() && it->second)
            {
                it->second(source_id, data);
            }
        }

        void Dashboard::initialize()
        {
            std::cout << "Initializing dashboard: " << title_ << "\n";
            std::cout << "Size: " << width_ << "x" << height_ << "\n";
            std::cout << "Panels: " << panels_.size() << "\n";
            std::cout << "Auto refresh: " << (auto_refresh_ ? "enabled" : "disabled") << "\n";
            if (auto_refresh_)
            {
                std::cout << "Refresh interval: " << refresh_interval_.count() << "ms\n";
            }
            std::cout << "\n";
        }

        void Dashboard::update()
        {
            for (auto &panel : panels_)
            {
                panel->update();
            }

            if (auto_refresh_)
            {
                auto now = std::chrono::system_clock::now();
                if (now - last_update_ >= refresh_interval_)
                {
                    last_update_ = now;
                    // Trigger refresh of all widgets
                    for (auto &panel : panels_)
                    {
                        for (auto &widget : panel->widgets())
                        {
                            widget->mark_for_update();
                        }
                    }
                }
            }
        }

        void Dashboard::render()
        {
            std::cout << "=== " << title_ << " ===\n";
            std::cout << "Dashboard rendering at " << width_ << "x" << height_ << "\n\n";

            for (auto &panel : panels_)
            {
                panel->render();
            }
        }

        void Dashboard::export_layout(const std::string &filename)
        {
            std::ofstream file(filename);
            if (!file.is_open())
            {
                return;
            }

            file << "{\n";
            file << "  \"title\": \"" << title_ << "\",\n";
            file << "  \"width\": " << width_ << ",\n";
            file << "  \"height\": " << height_ << ",\n";
            file << "  \"panels\": [\n";

            for (size_t i = 0; i < panels_.size(); ++i)
            {
                const auto &panel = panels_[i];
                file << "    {\n";
                file << "      \"id\": \"" << panel->id() << "\",\n";
                file << "      \"title\": \"" << panel->title() << "\",\n";
                file << "      \"x\": " << panel->x() << ",\n";
                file << "      \"y\": " << panel->y() << ",\n";
                file << "      \"width\": " << panel->width() << ",\n";
                file << "      \"height\": " << panel->height() << ",\n";
                file << "      \"visible\": " << (panel->visible() ? "true" : "false") << "\n";
                file << "    }";
                if (i < panels_.size() - 1)
                    file << ",";
                file << "\n";
            }

            file << "  ]\n";
            file << "}";
            file.close();
        }

        void Dashboard::load_layout(const std::string &filename)
        {
            std::ifstream file(filename);
            if (!file.is_open())
            {
                return;
            }

            // Simple JSON parsing for demo purposes
            // In a real implementation, you'd use a proper JSON parser
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            file.close();

            // Parse layout data
            if (content.find("title") != std::string::npos)
            {
                // Would parse layout data
            }
        }

        void Dashboard::clear()
        {
            panels_.clear();
            data_sources_.clear();
        }

        bool Dashboard::is_valid() const
        {
            return !title_.empty() && width_ > 0 && height_ > 0;
        }

        void Dashboard::auto_layout_panels()
        {
            // Simple auto-layout algorithm
            int panel_width = width_ / 2;
            int panel_height = height_ / 2;

            for (size_t i = 0; i < panels_.size(); ++i)
            {
                auto &panel = panels_[i];
                int row = i / 2;
                int col = i % 2;
                panel->set_position(col * panel_width, row * panel_height);
                panel->set_size(panel_width, panel_height);
            }
        }

        void Dashboard::handle_widget_resize(const std::string &widget_id, int new_width, int new_height)
        {
            auto widget = get_widget(widget_id);
            if (widget)
            {
                widget->set_size(new_width, new_height);
            }
        }

        void Dashboard::handle_widget_move(const std::string &widget_id, int new_x, int new_y)
        {
            auto widget = get_widget(widget_id);
            if (widget)
            {
                widget->set_position(new_x, new_y);
            }
        }

        // DashboardFactory implementation
        std::unique_ptr<Dashboard> DashboardFactory::create_dashboard(LayoutType type, const std::string &title)
        {
            auto dashboard = std::make_unique<Dashboard>(title);

            switch (type)
            {
            case LayoutType::TRADING_DESK:
            {
                // Create trading desk layout
                auto chart_panel = std::make_unique<DashboardPanel>("chart_panel", "Charts", 0, 0, 800, 600);
                auto order_panel = std::make_unique<DashboardPanel>("order_panel", "Order Book", 800, 0, 400, 300);
                auto position_panel = std::make_unique<DashboardPanel>("position_panel", "Positions", 800, 300, 400, 300);
                auto ticker_panel = std::make_unique<DashboardPanel>("ticker_panel", "Price Tickers", 0, 600, 1200, 200);

                dashboard->add_panel(std::move(chart_panel));
                dashboard->add_panel(std::move(order_panel));
                dashboard->add_panel(std::move(position_panel));
                dashboard->add_panel(std::move(ticker_panel));
                break;
            }
            case LayoutType::PORTFOLIO_OVERVIEW:
            {
                // Create portfolio overview layout
                auto pnl_panel = std::make_unique<DashboardPanel>("pnl_panel", "P&L Chart", 0, 0, 600, 400);
                auto metrics_panel = std::make_unique<DashboardPanel>("metrics_panel", "Performance Metrics", 600, 0, 400, 400);
                auto position_panel = std::make_unique<DashboardPanel>("position_panel", "Positions", 0, 400, 1000, 300);

                dashboard->add_panel(std::move(pnl_panel));
                dashboard->add_panel(std::move(metrics_panel));
                dashboard->add_panel(std::move(position_panel));
                break;
            }
            case LayoutType::RISK_MANAGEMENT:
            {
                // Create risk management layout
                auto risk_panel = std::make_unique<DashboardPanel>("risk_panel", "Risk Metrics", 0, 0, 500, 400);
                auto exposure_panel = std::make_unique<DashboardPanel>("exposure_panel", "Exposure", 500, 0, 500, 400);
                auto alerts_panel = std::make_unique<DashboardPanel>("alerts_panel", "Alerts", 0, 400, 1000, 300);

                dashboard->add_panel(std::move(risk_panel));
                dashboard->add_panel(std::move(exposure_panel));
                dashboard->add_panel(std::move(alerts_panel));
                break;
            }
            case LayoutType::PERFORMANCE_ANALYSIS:
            {
                // Create performance analysis layout
                auto performance_panel = std::make_unique<DashboardPanel>("performance_panel", "Performance", 0, 0, 600, 400);
                auto drawdown_panel = std::make_unique<DashboardPanel>("drawdown_panel", "Drawdown", 600, 0, 400, 400);
                auto metrics_panel = std::make_unique<DashboardPanel>("metrics_panel", "Metrics", 0, 400, 1000, 300);

                dashboard->add_panel(std::move(performance_panel));
                dashboard->add_panel(std::move(drawdown_panel));
                dashboard->add_panel(std::move(metrics_panel));
                break;
            }
            case LayoutType::CUSTOM:
            default:
                // Custom layout - empty dashboard
                break;
            }

            return dashboard;
        }

    } // namespace visualization
} // namespace trading
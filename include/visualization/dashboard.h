#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <chrono>
#include <atomic>

#include "../data/market_data.h"
#include "../data/data_processor.h"
#include "chart_renderer.h"

namespace trading
{
    namespace visualization
    {

        // Forward declarations
        class DashboardWidget;
        class DashboardPanel;

        // Widget types
        enum class WidgetType
        {
            PRICE_TICKER,
            CHART,
            ORDER_BOOK,
            POSITION_SUMMARY,
            PNL_CHART,
            PERFORMANCE_METRICS,
            NEWS_FEED,
            ALERTS,
            VOLUME_PROFILE,
            TECHNICAL_INDICATORS
        };

        // Widget configuration
        struct WidgetConfig
        {
            WidgetType type;
            std::string title;
            int x, y, width, height;
            bool visible = true;
            bool resizable = true;
            bool draggable = true;
            std::map<std::string, std::string> properties;

            WidgetConfig(WidgetType t, const std::string &tit, int x_pos, int y_pos, int w, int h)
                : type(t), title(tit), x(x_pos), y(y_pos), width(w), height(h) {}
        };

        // Real-time data update callback
        using DataUpdateCallback = std::function<void(const std::string &widget_id, const std::string &data)>;

        // Base widget class
        class DashboardWidget
        {
        protected:
            std::string id_;
            WidgetConfig config_;
            std::atomic<bool> needs_update_;
            DataUpdateCallback update_callback_;

        public:
            DashboardWidget(const std::string &widget_id, const WidgetConfig &cfg)
                : id_(widget_id), config_(cfg), needs_update_(false) {}

            virtual ~DashboardWidget() = default;

            // Getters
            const std::string &id() const { return id_; }
            const WidgetConfig &config() const { return config_; }
            bool needs_update() const { return needs_update_; }

            // Setters
            void set_update_callback(DataUpdateCallback callback) { update_callback_ = callback; }
            void set_visible(bool visible) { config_.visible = visible; }
            void set_position(int x, int y)
            {
                config_.x = x;
                config_.y = y;
            }
            void set_size(int width, int height)
            {
                config_.width = width;
                config_.height = height;
            }

            // Virtual methods
            virtual void update() = 0;
            virtual void render() = 0;
            virtual std::string get_data() const = 0;
            virtual void set_data(const std::string &data) = 0;

            // Mark for update
            void mark_for_update();
            void clear_update_flag();
        };

        // Price ticker widget
        class PriceTickerWidget : public DashboardWidget
        {
        private:
            std::string symbol_;
            double current_price_;
            double price_change_;
            double price_change_percent_;
            uint64_t volume_;
            std::chrono::system_clock::time_point last_update_;

        public:
            PriceTickerWidget(const std::string &widget_id, const WidgetConfig &cfg, const std::string &symbol)
                : DashboardWidget(widget_id, cfg), symbol_(symbol), current_price_(0.0),
                  price_change_(0.0), price_change_percent_(0.0), volume_(0) {}

            void update() override;
            void render() override;
            std::string get_data() const override;
            void set_data(const std::string &data) override;

            // Specific methods
            void update_price(double price, double change, double change_percent, uint64_t volume);
            const std::string &symbol() const { return symbol_; }

        private:
            // Utility functions
            std::string format_timestamp(const std::chrono::system_clock::time_point &timestamp) const;
            std::string format_volume(uint64_t volume) const;
        };

        // Chart widget
        class ChartWidget : public DashboardWidget
        {
        private:
            std::unique_ptr<ChartRenderer> renderer_;
            std::vector<CandlestickPoint> candlestick_data_;
            std::vector<IndicatorOverlay> indicators_;
            std::vector<ChartSeries> current_series_;
            ChartConfig chart_config_;

        public:
            ChartWidget(const std::string &widget_id, const WidgetConfig &cfg,
                        std::unique_ptr<ChartRenderer> renderer);

            void update() override;
            void render() override;
            std::string get_data() const override;
            void set_data(const std::string &data) override;

            // Specific methods
            void update_candlestick_data(const std::vector<CandlestickPoint> &data);
            void update_indicators(const std::vector<IndicatorOverlay> &indicators);
            void set_chart_config(const ChartConfig &config);
            bool export_chart(const std::string &filename);
        };

        // Order book widget
        class OrderBookWidget : public DashboardWidget
        {
        private:
            struct OrderBookLevel
            {
                double price;
                uint64_t quantity;
                int order_count;
                bool is_bid;

                OrderBookLevel(double p, uint64_t q, int count, bool bid)
                    : price(p), quantity(q), order_count(count), is_bid(bid) {}
            };

            std::vector<OrderBookLevel> bids_;
            std::vector<OrderBookLevel> asks_;
            double spread_;
            uint64_t total_bid_volume_;
            uint64_t total_ask_volume_;

        public:
            OrderBookWidget(const std::string &widget_id, const WidgetConfig &cfg)
                : DashboardWidget(widget_id, cfg), spread_(0.0), total_bid_volume_(0), total_ask_volume_(0) {}

            void update() override;
            void render() override;
            std::string get_data() const override;
            void set_data(const std::string &data) override;

            // Specific methods
            void update_order_book(const std::vector<OrderBookLevel> &bids,
                                   const std::vector<OrderBookLevel> &asks);
            double spread() const { return spread_; }
            uint64_t total_bid_volume() const { return total_bid_volume_; }
            uint64_t total_ask_volume() const { return total_ask_volume_; }

        private:
            // Utility functions
            std::string format_volume(uint64_t volume);
        };

        // Position summary widget
        class PositionSummaryWidget : public DashboardWidget
        {
        public:
            struct Position
            {
                std::string symbol;
                double quantity;
                double avg_price;
                double current_price;
                double unrealized_pnl;
                double realized_pnl;

                Position(const std::string &sym, double qty, double avg, double current)
                    : symbol(sym), quantity(qty), avg_price(avg), current_price(current),
                      unrealized_pnl(0.0), realized_pnl(0.0) {}
            };

        private:
            std::vector<Position> positions_;
            double total_unrealized_pnl_;
            double total_realized_pnl_;
            double total_portfolio_value_;

        public:
            PositionSummaryWidget(const std::string &widget_id, const WidgetConfig &cfg)
                : DashboardWidget(widget_id, cfg), total_unrealized_pnl_(0.0),
                  total_realized_pnl_(0.0), total_portfolio_value_(0.0) {}

            void update() override;
            void render() override;
            std::string get_data() const override;
            void set_data(const std::string &data) override;

            // Specific methods
            void update_positions(const std::vector<Position> &positions);
            void update_portfolio_value(double value);
            double total_pnl() const { return total_unrealized_pnl_ + total_realized_pnl_; }
        };

        // P&L chart widget
        class PnLChartWidget : public DashboardWidget
        {
        private:
            std::unique_ptr<ChartRenderer> renderer_;
            std::vector<ChartPoint> pnl_data_;
            std::vector<ChartPoint> drawdown_data_;
            ChartConfig chart_config_;
            double max_drawdown_;
            double total_return_;

        public:
            PnLChartWidget(const std::string &widget_id, const WidgetConfig &cfg,
                           std::unique_ptr<ChartRenderer> renderer);

            void update() override;
            void render() override;
            std::string get_data() const override;
            void set_data(const std::string &data) override;

            // Specific methods
            void update_pnl_data(const std::vector<ChartPoint> &pnl, const std::vector<ChartPoint> &drawdown);
            void update_metrics(double max_dd, double total_ret);
            double max_drawdown() const { return max_drawdown_; }
            double total_return() const { return total_return_; }
        };

        // Performance metrics widget
        class PerformanceMetricsWidget : public DashboardWidget
        {
        private:
            struct Metrics
            {
                double sharpe_ratio;
                double sortino_ratio;
                double calmar_ratio;
                double max_drawdown;
                double total_return;
                double annualized_return;
                double volatility;
                double win_rate;
                int total_trades;
                double avg_win;
                double avg_loss;
                double profit_factor;
            };

            Metrics metrics_;

        public:
            PerformanceMetricsWidget(const std::string &widget_id, const WidgetConfig &cfg)
                : DashboardWidget(widget_id, cfg)
            {
                // Initialize metrics to zero
                std::memset(&metrics_, 0, sizeof(metrics_));
            }

            void update() override;
            void render() override;
            std::string get_data() const override;
            void set_data(const std::string &data) override;

            // Specific methods
            void update_metrics(const Metrics &metrics);
            const Metrics &get_metrics() const { return metrics_; }
        };

        // Dashboard panel (container for widgets)
        class DashboardPanel
        {
        private:
            std::string id_;
            std::string title_;
            int x_, y_, width_, height_;
            std::vector<std::unique_ptr<DashboardWidget>> widgets_;
            bool visible_;

        public:
            DashboardPanel(const std::string &panel_id, const std::string &title,
                           int x, int y, int w, int h)
                : id_(panel_id), title_(title), x_(x), y_(y), width_(w), height_(h), visible_(true) {}

            // Widget management
            void add_widget(std::unique_ptr<DashboardWidget> widget);
            void remove_widget(const std::string &widget_id);
            DashboardWidget *get_widget(const std::string &widget_id);

            // Panel management
            void set_position(int x, int y)
            {
                x_ = x;
                y_ = y;
            }
            void set_size(int width, int height)
            {
                width_ = width;
                height_ = height;
            }
            void set_visible(bool visible) { visible_ = visible; }

            // Getters
            const std::string &id() const { return id_; }
            const std::string &title() const { return title_; }
            int x() const { return x_; }
            int y() const { return y_; }
            int width() const { return width_; }
            int height() const { return height_; }
            bool visible() const { return visible_; }
            const std::vector<std::unique_ptr<DashboardWidget>> &widgets() const { return widgets_; }

            // Rendering
            void render();
            void update();
        };

        // Main dashboard class
        class Dashboard
        {
        private:
            std::string title_;
            int width_, height_;
            std::vector<std::unique_ptr<DashboardPanel>> panels_;
            std::map<std::string, DataUpdateCallback> data_sources_;
            std::chrono::system_clock::time_point last_update_;
            bool auto_refresh_;
            std::chrono::milliseconds refresh_interval_;

            // Layout management
            void auto_layout_panels();
            void handle_widget_resize(const std::string &widget_id, int new_width, int new_height);
            void handle_widget_move(const std::string &widget_id, int new_x, int new_y);

        public:
            Dashboard(const std::string &title, int width = 1920, int height = 1080)
                : title_(title), width_(width), height_(height), auto_refresh_(true),
                  refresh_interval_(std::chrono::milliseconds(1000)) {}

            // Panel management
            void add_panel(std::unique_ptr<DashboardPanel> panel);
            void remove_panel(const std::string &panel_id);
            DashboardPanel *get_panel(const std::string &panel_id);

            // Widget management (convenience methods)
            void add_widget_to_panel(const std::string &panel_id, std::unique_ptr<DashboardWidget> widget);
            DashboardWidget *get_widget(const std::string &widget_id);

            // Data source management
            void register_data_source(const std::string &source_id, DataUpdateCallback callback);
            void unregister_data_source(const std::string &source_id);
            void update_data_source(const std::string &source_id, const std::string &data);

            // Configuration
            void set_auto_refresh(bool enabled) { auto_refresh_ = enabled; }
            void set_refresh_interval(std::chrono::milliseconds interval) { refresh_interval_ = interval; }
            void set_size(int width, int height)
            {
                width_ = width;
                height_ = height;
            }

            // Getters
            const std::string &title() const { return title_; }
            int width() const { return width_; }
            int height() const { return height_; }
            bool auto_refresh() const { return auto_refresh_; }
            std::chrono::milliseconds refresh_interval() const { return refresh_interval_; }

            // Main operations
            void initialize();
            void update();
            void render();
            void export_layout(const std::string &filename);
            void load_layout(const std::string &filename);

            // Utility
            void clear();
            bool is_valid() const;
        };

        // Dashboard factory for creating common layouts
        class DashboardFactory
        {
        public:
            enum class LayoutType
            {
                TRADING_DESK,
                PORTFOLIO_OVERVIEW,
                RISK_MANAGEMENT,
                PERFORMANCE_ANALYSIS,
                CUSTOM
            };

            static std::unique_ptr<Dashboard> create_dashboard(LayoutType type,
                                                               const std::string &title = "Trading Dashboard");
        };

    } // namespace visualization
} // namespace trading
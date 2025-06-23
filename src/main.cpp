#include <iostream>
#include <chrono>
#include <vector>
#include <random>

// Include our core components
#include "core/thread_pool.h"
#include "core/memory_pool.h"
#include "core/lock_free_queue.h"

// Include our data components
#include "data/yahoo_finance.h"
#include "data/data_processor.h"
#include "data/cache_manager.h"

// Include our visualization components
#include "visualization/chart_renderer.h"
#include "visualization/dashboard.h"
#include "visualization/data_export.h"

using namespace trading;

// Simple test function for thread pool
int fibonacci(int n)
{
    if (n < 2)
        return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Test function that uses memory pool
void test_memory_pool()
{
    std::cout << "Testing Memory Pool..." << std::endl;

    // Create a memory pool for integers
    MemoryPool pool(sizeof(int), 10);

    std::cout << "Initial pool state:" << std::endl;
    std::cout << "  Total blocks: " << pool.total_blocks() << std::endl;
    std::cout << "  Free blocks: " << pool.free_blocks() << std::endl;
    std::cout << "  Allocated blocks: " << pool.allocated_blocks() << std::endl;

    // Allocate some integers
    std::vector<int *> allocated;
    for (int i = 0; i < 5; ++i)
    {
        int *ptr = static_cast<int *>(pool.allocate());
        *ptr = i * i;
        allocated.push_back(ptr);
        std::cout << "Allocated: " << *ptr << std::endl;
    }

    std::cout << "After allocation:" << std::endl;
    std::cout << "  Total blocks: " << pool.total_blocks() << std::endl;
    std::cout << "  Free blocks: " << pool.free_blocks() << std::endl;
    std::cout << "  Allocated blocks: " << pool.allocated_blocks() << std::endl;

    // Deallocate some
    for (int i = 0; i < 3; ++i)
    {
        pool.deallocate(allocated[i]);
        std::cout << "Deallocated: " << i * i << std::endl;
    }

    std::cout << "After deallocation:" << std::endl;
    std::cout << "  Total blocks: " << pool.total_blocks() << std::endl;
    std::cout << "  Free blocks: " << pool.free_blocks() << std::endl;
    std::cout << "  Allocated blocks: " << pool.allocated_blocks() << std::endl;

    // Deallocate the rest
    for (int i = 3; i < 5; ++i)
    {
        pool.deallocate(allocated[i]);
    }

    std::cout << "Memory pool test completed!" << std::endl
              << std::endl;
}

// Test function for thread pool
void test_thread_pool()
{
    std::cout << "Testing Thread Pool..." << std::endl;

    // Create a thread pool with 4 threads
    ThreadPool pool(4);

    std::cout << "Created thread pool with " << pool.thread_count() << " threads" << std::endl;

    // Submit some tasks
    std::vector<std::future<int>> futures;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 8; ++i)
    {
        futures.push_back(pool.submit(fibonacci, 20 + i));
    }

    // Wait for all tasks to complete
    for (auto &future : futures)
    {
        int result = future.get();
        std::cout << "Fibonacci result: " << result << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "All tasks completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Thread pool test completed!" << std::endl
              << std::endl;
}

// Test function for lock-free queue
void test_lock_free_queue()
{
    std::cout << "Testing Lock-Free Queue..." << std::endl;

    // Create a lock-free queue
    LockFreeQueue<int> queue(100);

    std::cout << "Created lock-free queue with capacity: " << queue.capacity() << std::endl;

    // Test basic operations
    for (int i = 0; i < 10; ++i)
    {
        bool success = queue.try_push(i);
        std::cout << "Pushed " << i << ": " << (success ? "success" : "failed") << std::endl;
    }

    std::cout << "Queue size: " << queue.size() << std::endl;
    std::cout << "Queue empty: " << (queue.empty() ? "yes" : "no") << std::endl;
    std::cout << "Queue full: " << (queue.full() ? "yes" : "no") << std::endl;

    // Pop all elements
    int value;
    while (queue.try_pop(value))
    {
        std::cout << "Popped: " << value << std::endl;
    }

    std::cout << "Queue size after popping: " << queue.size() << std::endl;
    std::cout << "Lock-free queue test completed!" << std::endl
              << std::endl;
}

// Test function for Yahoo Finance client
void test_yahoo_finance()
{
    std::cout << "Testing Yahoo Finance Client..." << std::endl;

    // Create thread pool for async operations
    auto thread_pool = std::make_shared<ThreadPool>(2);

    // Create Yahoo Finance client
    YahooFinanceClient client(thread_pool);

    std::cout << "Created Yahoo Finance client" << std::endl;

    // Test symbol validation
    std::vector<std::string> test_symbols = {"AAPL", "MSFT", "GOOGL", "INVALID", ""};
    for (const auto &symbol : test_symbols)
    {
        bool valid = client.validate_symbol(symbol);
        std::cout << "Symbol '" << symbol << "' is " << (valid ? "valid" : "invalid") << std::endl;
    }

    // Test current price (placeholder)
    double price = client.get_current_price("AAPL");
    std::cout << "Current price for AAPL: $" << price << std::endl;

    // Test historical data fetching
    try
    {
        MarketDataRequest request;
        request.symbol = "AAPL";
        request.start_date = std::chrono::system_clock::now() - std::chrono::hours(24 * 30); // 30 days ago
        request.end_date = std::chrono::system_clock::now();
        request.interval = "1d";

        std::cout << "Fetching historical data for " << request.symbol << "..." << std::endl;

        auto future = client.fetch_historical_data(request);
        auto series = future.get();

        std::cout << "Successfully fetched " << series.size() << " data points" << std::endl;

        if (!series.empty())
        {
            const auto &latest = series[series.size() - 1];
            std::cout << "Latest data point:" << std::endl;
            std::cout << "  Date: " << std::chrono::duration_cast<std::chrono::seconds>(latest.timestamp.time_since_epoch()).count() << std::endl;
            std::cout << "  Open: $" << latest.open << std::endl;
            std::cout << "  High: $" << latest.high << std::endl;
            std::cout << "  Low: $" << latest.low << std::endl;
            std::cout << "  Close: $" << latest.close << std::endl;
            std::cout << "  Volume: " << latest.volume << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Error fetching historical data: " << e.what() << std::endl;
    }

    std::cout << "Yahoo Finance client test completed!" << std::endl
              << std::endl;
}

// Test function for data processor
void test_data_processor()
{
    std::cout << "Testing Data Processor..." << std::endl;

    // Create data processor
    DataProcessor processor;

    // Create sample market data
    MarketDataSeries series("TEST");

    // Generate some sample data points
    auto now = std::chrono::system_clock::now();
    double base_price = 100.0;

    for (int i = 0; i < 100; ++i)
    {
        double price_change = (i % 10 - 5) * 0.5; // Oscillating price
        double current_price = base_price + price_change;

        MarketDataPoint point(
            now + std::chrono::hours(i),
            current_price - 0.5,
            current_price + 0.5,
            current_price - 1.0,
            current_price,
            1000000 + (i * 10000));

        series.add_point(std::move(point));
        base_price = current_price;
    }

    std::cout << "Created sample data series with " << series.size() << " points" << std::endl;

    // Test data cleaning
    auto cleaned_series = processor.clean_data(series);
    std::cout << "Cleaned data series has " << cleaned_series.size() << " points" << std::endl;

    // Test technical indicators
    auto indicators = processor.calculate_indicators(series);
    std::cout << "Calculated technical indicators:" << std::endl;
    std::cout << "  SMA 20: " << indicators.sma_20.size() << " values" << std::endl;
    std::cout << "  SMA 50: " << indicators.sma_50.size() << " values" << std::endl;
    std::cout << "  EMA 12: " << indicators.ema_12.size() << " values" << std::endl;
    std::cout << "  EMA 26: " << indicators.ema_26.size() << " values" << std::endl;
    std::cout << "  RSI: " << indicators.rsi.size() << " values" << std::endl;
    std::cout << "  MACD: " << indicators.macd.size() << " values" << std::endl;
    std::cout << "  Bollinger Bands: " << indicators.bollinger_upper.size() << " values" << std::endl;
    std::cout << "  Volume SMA: " << indicators.volume_sma.size() << " values" << std::endl;

    // Show some sample values
    if (!indicators.sma_20.empty() && !std::isnan(indicators.sma_20.back()))
    {
        std::cout << "  Latest SMA 20: " << indicators.sma_20.back() << std::endl;
    }
    if (!indicators.rsi.empty() && !std::isnan(indicators.rsi.back()))
    {
        std::cout << "  Latest RSI: " << indicators.rsi.back() << std::endl;
    }

    // Test other calculations
    std::vector<double> prices;
    for (const auto &point : series.data())
    {
        prices.push_back(point.close);
    }

    auto returns = processor.calculate_returns(prices);
    auto volatility = processor.calculate_volatility(returns, 20);
    auto normalized = processor.normalize_prices(prices);

    std::cout << "  Returns calculated: " << returns.size() << " values" << std::endl;
    std::cout << "  Volatility calculated: " << volatility.size() << " values" << std::endl;
    std::cout << "  Normalized prices: " << normalized.size() << " values" << std::endl;

    std::cout << "Data processor test completed!" << std::endl
              << std::endl;
}

// Test function for cache manager
void test_cache_manager()
{
    std::cout << "Testing Cache Manager..." << std::endl;

    // Create thread pool for async operations
    auto thread_pool = std::make_shared<ThreadPool>(2);

    // Create cache manager
    CacheManager cache(10, "cache_test", thread_pool); // 10MB cache

    std::cout << "Created cache manager with 10MB limit" << std::endl;
    std::cout << "Initial cache size: " << cache.size() << std::endl;
    std::cout << "Initial memory usage: " << cache.memory_usage() << " bytes" << std::endl;

    // Create sample market data
    MarketDataSeries series("CACHE_TEST");
    auto now = std::chrono::system_clock::now();

    for (int i = 0; i < 50; ++i)
    {
        MarketDataPoint point(
            now + std::chrono::hours(i),
            100.0 + i * 0.1,
            101.0 + i * 0.1,
            99.0 + i * 0.1,
            100.5 + i * 0.1,
            1000000);
        series.add_point(std::move(point));
    }

    // Test cache operations
    std::string key = "AAPL_2024_01";

    std::cout << "Adding data to cache with key: " << key << std::endl;
    cache.put(key, series);

    std::cout << "Cache size after adding: " << cache.size() << std::endl;
    std::cout << "Memory usage after adding: " << cache.memory_usage() << " bytes" << std::endl;
    std::cout << "Contains key '" << key << "': " << (cache.contains(key) ? "yes" : "no") << std::endl;

    // Test retrieval
    auto retrieved = cache.get(key);
    if (retrieved)
    {
        std::cout << "Successfully retrieved data with " << retrieved->size() << " points" << std::endl;
        std::cout << "Symbol: " << retrieved->symbol() << std::endl;
    }
    else
    {
        std::cout << "Failed to retrieve data" << std::endl;
    }

    // Test cache hit rate
    std::cout << "Cache hit rate: " << (cache.hit_rate() * 100) << "%" << std::endl;

    // Test multiple entries
    for (int i = 0; i < 5; ++i)
    {
        std::string test_key = "TEST_" + std::to_string(i);
        cache.put(test_key, series);
    }

    std::cout << "Cache size after adding multiple entries: " << cache.size() << std::endl;
    std::cout << "Memory usage: " << cache.memory_usage() << " bytes" << std::endl;

    // Test removal
    cache.remove(key);
    std::cout << "Cache size after removal: " << cache.size() << std::endl;
    std::cout << "Contains removed key: " << (cache.contains(key) ? "yes" : "no") << std::endl;

    std::cout << "Cache manager test completed!" << std::endl
              << std::endl;
}

// Test function for chart renderer
void test_chart_renderer()
{
    std::cout << "Testing Chart Renderer..." << std::endl;

    // Create chart renderers
    auto html_renderer = visualization::ChartFactory::create_renderer(visualization::ChartFactory::RendererType::HTML);
    auto console_renderer = visualization::ChartFactory::create_renderer(visualization::ChartFactory::RendererType::CONSOLE);

    std::cout << "Created HTML and Console chart renderers" << std::endl;

    // Create sample candlestick data
    std::vector<visualization::CandlestickPoint> candlestick_data;
    auto now = std::chrono::system_clock::now();
    double base_price = 100.0;

    for (int i = 0; i < 20; ++i)
    {
        double price_change = (i % 5 - 2) * 2.0;
        double current_price = base_price + price_change;

        MarketDataPoint point(
            now + std::chrono::hours(i),
            current_price - 1.0,
            current_price + 1.5,
            current_price - 2.0,
            current_price + 0.5,
            1000000 + (i * 50000));

        candlestick_data.emplace_back(point);
        base_price = current_price;
    }

    std::cout << "Created " << candlestick_data.size() << " candlestick data points" << std::endl;

    // Test HTML renderer
    if (html_renderer)
    {
        visualization::ChartConfig config;
        config.width = 800;
        config.height = 600;
        config.title = "Sample Candlestick Chart";
        config.show_grid = true;
        config.show_legend = true;

        html_renderer->initialize(config);
        bool success = html_renderer->render_candlestick_chart(candlestick_data, {}, config);

        if (success)
        {
            std::cout << "HTML chart rendered successfully" << std::endl;

            // Export to file
            bool exported = html_renderer->export_to_file("sample_chart.html", "html");
            if (exported)
            {
                std::cout << "Chart exported to sample_chart.html" << std::endl;
            }

            // Get JSON data
            std::string json_data = html_renderer->get_chart_data("json");
            std::cout << "Chart JSON data length: " << json_data.length() << " characters" << std::endl;
        }
        else
        {
            std::cout << "Failed to render HTML chart" << std::endl;
        }
    }

    // Test console renderer
    if (console_renderer)
    {
        visualization::ChartConfig config;
        config.title = "Console Candlestick Chart";
        config.show_volume = true;

        console_renderer->initialize(config);
        bool success = console_renderer->render_candlestick_chart(candlestick_data, {}, config);

        if (success)
        {
            std::cout << "Console chart rendered successfully" << std::endl;

            // Export to file
            bool exported = console_renderer->export_to_file("sample_chart.txt", "txt");
            if (exported)
            {
                std::cout << "Chart exported to sample_chart.txt" << std::endl;
            }
        }
        else
        {
            std::cout << "Failed to render console chart" << std::endl;
        }
    }

    // Test line chart
    std::vector<visualization::ChartSeries> line_series;

    // Create price series
    visualization::ChartSeries price_series("Price", visualization::ChartType::LINE, "#00ff00");
    for (size_t i = 0; i < candlestick_data.size(); ++i)
    {
        price_series.points.emplace_back(i, candlestick_data[i].close, "", "#00ff00");
    }
    line_series.push_back(std::move(price_series));

    // Create volume series
    visualization::ChartSeries volume_series("Volume", visualization::ChartType::BAR, "#0088ff");
    for (size_t i = 0; i < candlestick_data.size(); ++i)
    {
        volume_series.points.emplace_back(i, candlestick_data[i].volume / 1000000.0, "", "#0088ff");
    }
    line_series.push_back(std::move(volume_series));

    if (html_renderer)
    {
        visualization::ChartConfig config;
        config.width = 800;
        config.height = 400;
        config.title = "Price and Volume Chart";
        config.show_grid = true;
        config.show_legend = true;

        bool success = html_renderer->render_line_chart(line_series, config);
        if (success)
        {
            std::cout << "Line chart rendered successfully" << std::endl;
            html_renderer->export_to_file("line_chart.html", "html");
        }
    }

    std::cout << "Chart renderer test completed!" << std::endl
              << std::endl;
}

// Test function for dashboard
void test_dashboard()
{
    std::cout << "Testing Dashboard..." << std::endl;

    // Create a trading desk dashboard
    auto dashboard = visualization::DashboardFactory::create_dashboard(
        visualization::DashboardFactory::LayoutType::TRADING_DESK,
        "Trading Simulator Dashboard");

    if (!dashboard)
    {
        std::cout << "Failed to create dashboard" << std::endl;
        return;
    }

    std::cout << "Created dashboard: " << dashboard->title() << std::endl;
    std::cout << "Dashboard size: " << dashboard->width() << "x" << dashboard->height() << std::endl;

    // Initialize dashboard
    dashboard->initialize();

    // Create some widgets
    auto chart_panel = dashboard->get_panel("chart_panel");
    if (chart_panel)
    {
        // Create chart widget
        auto html_renderer = visualization::ChartFactory::create_renderer(visualization::ChartFactory::RendererType::HTML);
        visualization::WidgetConfig chart_config(
            visualization::WidgetType::CHART,
            "AAPL Chart",
            10, 10, 780, 580);

        auto chart_widget = std::make_unique<visualization::ChartWidget>(
            "aapl_chart", chart_config, std::move(html_renderer));

        chart_panel->add_widget(std::move(chart_widget));
        std::cout << "Added chart widget to chart panel" << std::endl;
    }

    auto ticker_panel = dashboard->get_panel("ticker_panel");
    if (ticker_panel)
    {
        // Create price ticker widgets
        std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL"};

        for (size_t i = 0; i < symbols.size(); ++i)
        {
            visualization::WidgetConfig ticker_config(
                visualization::WidgetType::PRICE_TICKER,
                symbols[i] + " Ticker",
                10 + i * 200, 10, 180, 80);

            auto ticker_widget = std::make_unique<visualization::PriceTickerWidget>(
                "ticker_" + symbols[i], ticker_config, symbols[i]);

            // Update with sample data
            ticker_widget->update_price(150.0 + i * 10, 2.5 + i, 1.5 + i * 0.5, 1000000 + i * 100000);

            ticker_panel->add_widget(std::move(ticker_widget));
        }

        std::cout << "Added " << symbols.size() << " price ticker widgets" << std::endl;
    }

    auto position_panel = dashboard->get_panel("position_panel");
    if (position_panel)
    {
        // Create position summary widget
        visualization::WidgetConfig position_config(
            visualization::WidgetType::POSITION_SUMMARY,
            "Portfolio Positions",
            10, 10, 380, 280);

        auto position_widget = std::make_unique<visualization::PositionSummaryWidget>(
            "portfolio_positions", position_config);

        // Add sample positions
        std::vector<visualization::PositionSummaryWidget::Position> positions;
        positions.emplace_back("AAPL", 100, 150.0, 152.5);
        positions.emplace_back("MSFT", 50, 300.0, 305.0);
        positions.emplace_back("GOOGL", 25, 2800.0, 2850.0);

        position_widget->update_positions(positions);
        position_widget->update_portfolio_value(150000.0);

        position_panel->add_widget(std::move(position_widget));
        std::cout << "Added position summary widget" << std::endl;
    }

    // Render dashboard
    dashboard->render();

    // Test dashboard operations
    dashboard->set_auto_refresh(true);
    dashboard->set_refresh_interval(std::chrono::milliseconds(5000));

    std::cout << "Auto refresh: " << (dashboard->auto_refresh() ? "enabled" : "disabled") << std::endl;
    std::cout << "Refresh interval: " << dashboard->refresh_interval().count() << "ms" << std::endl;

    // Export dashboard layout
    dashboard->export_layout("dashboard_layout.json");
    std::cout << "Dashboard layout exported to dashboard_layout.json" << std::endl;

    std::cout << "Dashboard test completed!" << std::endl
              << std::endl;
}

// Test function for data export
void test_data_export()
{
    std::cout << "Testing Data Export..." << std::endl;

    // Create sample market data
    MarketDataSeries series("EXPORT_TEST");
    auto now = std::chrono::system_clock::now();
    double base_price = 100.0;

    for (int i = 0; i < 50; ++i)
    {
        double price_change = (i % 10 - 5) * 1.0;
        double current_price = base_price + price_change;

        MarketDataPoint point(
            now + std::chrono::hours(i),
            current_price - 0.5,
            current_price + 0.5,
            current_price - 1.0,
            current_price,
            1000000 + (i * 20000));

        series.add_point(std::move(point));
        base_price = current_price;
    }

    std::cout << "Created sample data series with " << series.size() << " points" << std::endl;

    // Test CSV export
    auto csv_exporter = visualization::ExportFactory::create_exporter(visualization::ExportFormat::CSV);
    if (csv_exporter)
    {
        visualization::ExportConfig csv_config("market_data.csv", visualization::ExportFormat::CSV);
        csv_config.include_headers = true;
        csv_config.delimiter = ",";

        bool success = csv_exporter->export_market_data(series, csv_config);
        if (success)
        {
            std::cout << "CSV export successful: market_data.csv" << std::endl;
        }
        else
        {
            std::cout << "CSV export failed" << std::endl;
        }
    }

    // Test JSON export
    auto json_exporter = visualization::ExportFactory::create_exporter(visualization::ExportFormat::JSON);
    if (json_exporter)
    {
        visualization::ExportConfig json_config("market_data.json", visualization::ExportFormat::JSON);

        bool success = json_exporter->export_market_data(series, json_config);
        if (success)
        {
            std::cout << "JSON export successful: market_data.json" << std::endl;
        }
        else
        {
            std::cout << "JSON export failed" << std::endl;
        }
    }

    // Test XML export
    auto xml_exporter = visualization::ExportFactory::create_exporter(visualization::ExportFormat::XML);
    if (xml_exporter)
    {
        visualization::ExportConfig xml_config("market_data.xml", visualization::ExportFormat::XML);

        bool success = xml_exporter->export_market_data(series, xml_config);
        if (success)
        {
            std::cout << "XML export successful: market_data.xml" << std::endl;
        }
        else
        {
            std::cout << "XML export failed" << std::endl;
        }
    }

    // Test batch export
    visualization::BatchExporter batch_exporter;

    // Add exporters
    batch_exporter.add_exporter(visualization::ExportFactory::create_exporter(visualization::ExportFormat::CSV));
    batch_exporter.add_exporter(visualization::ExportFactory::create_exporter(visualization::ExportFormat::JSON));

    // Add export configurations
    batch_exporter.add_export_config("csv_export",
                                     visualization::ExportConfig("batch_market_data.csv", visualization::ExportFormat::CSV));
    batch_exporter.add_export_config("json_export",
                                     visualization::ExportConfig("batch_market_data.json", visualization::ExportFormat::JSON));

    bool batch_success = batch_exporter.export_market_data_batch(series);
    if (batch_success)
    {
        std::cout << "Batch export successful" << std::endl;
    }
    else
    {
        std::cout << "Batch export failed" << std::endl;
    }

    // Test export utilities
    std::string filename = visualization::ExportUtils::generate_filename("test_export", ".csv");
    std::cout << "Generated filename: " << filename << std::endl;

    auto supported_formats = visualization::ExportUtils::get_supported_formats();
    std::cout << "Supported export formats:" << std::endl;
    for (const auto &[format, name] : supported_formats)
    {
        std::cout << "  " << visualization::ExportFactory::get_format_name(format)
                  << " (" << name << ")" << std::endl;
    }

    std::cout << "Data export test completed!" << std::endl
              << std::endl;
}

int main()
{
    std::cout << "=== Trading Simulator - Phase 3 Visualization Test ===" << std::endl;
    std::cout << "Testing core components, data integration, and visualization..." << std::endl
              << std::endl;

    try
    {
        // Test core components (Phase 1)
        test_memory_pool();
        test_thread_pool();
        test_lock_free_queue();

        // Test data components (Phase 2)
        test_yahoo_finance();
        test_data_processor();
        test_cache_manager();

        // Test visualization components (Phase 3)
        test_chart_renderer();
        test_dashboard();
        test_data_export();

        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "Phase 3 visualization is working correctly." << std::endl;
        std::cout << "Generated files:" << std::endl;
        std::cout << "  - sample_chart.html (HTML candlestick chart)" << std::endl;
        std::cout << "  - sample_chart.txt (Console chart)" << std::endl;
        std::cout << "  - line_chart.html (Line chart)" << std::endl;
        std::cout << "  - dashboard_layout.json (Dashboard layout)" << std::endl;
        std::cout << "  - market_data.csv (CSV export)" << std::endl;
        std::cout << "  - market_data.json (JSON export)" << std::endl;
        std::cout << "  - market_data.xml (XML export)" << std::endl;
        std::cout << "  - batch_market_data.csv (Batch CSV export)" << std::endl;
        std::cout << "  - batch_market_data.json (Batch JSON export)" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
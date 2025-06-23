#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

namespace trading
{

    /**
     * @brief Thread-safe thread pool for parallel task execution
     *
     * This class manages a pool of worker threads that can execute tasks in parallel.
     * It's designed for high-performance scenarios where you need to process many
     * independent tasks efficiently.
     *
     * Key features:
     * - Thread-safe task submission
     * - Automatic load balancing
     * - Graceful shutdown
     * - Exception safety
     */
    class ThreadPool
    {
    public:
        /**
         * @brief Constructor
         * @param num_threads Number of worker threads to create
         *
         * Creates a thread pool with the specified number of worker threads.
         * If num_threads is 0, uses the number of CPU cores.
         */
        explicit ThreadPool(size_t num_threads = 0);

        /**
         * @brief Destructor
         *
         * Ensures graceful shutdown of all worker threads.
         */
        ~ThreadPool();

        // Prevent copying
        ThreadPool(const ThreadPool &) = delete;
        ThreadPool &operator=(const ThreadPool &) = delete;

        // Allow moving
        ThreadPool(ThreadPool &&) noexcept;
        ThreadPool &operator=(ThreadPool &&) noexcept;

        /**
         * @brief Submit a task for execution
         * @param task Function to execute
         * @return Future containing the result
         *
         * Submits a task to the thread pool and returns a future that will
         * contain the result when the task completes.
         */
        template <typename F, typename... Args>
        auto submit(F &&task, Args &&...args)
            -> std::future<typename std::invoke_result_t<F, Args...>>;

        /**
         * @brief Get the number of worker threads
         * @return Number of threads in the pool
         */
        size_t thread_count() const { return workers_.size(); }

        /**
         * @brief Get the number of pending tasks
         * @return Number of tasks waiting to be executed
         */
        size_t pending_tasks() const;

        /**
         * @brief Wait for all tasks to complete
         *
         * Blocks until all submitted tasks have been executed.
         */
        void wait_all();

        /**
         * @brief Stop accepting new tasks and wait for completion
         *
         * Signals the thread pool to stop accepting new tasks and waits
         * for all currently queued tasks to complete.
         */
        void shutdown();

    private:
        // Worker thread function
        void worker_function();

        // Member variables
        std::vector<std::thread> workers_;        // Worker threads
        std::queue<std::function<void()>> tasks_; // Task queue

        // Synchronization primitives
        mutable std::mutex queue_mutex_;    // Protects task queue
        std::condition_variable condition_; // Signals workers
        std::atomic<bool> stop_;            // Shutdown flag

        // Statistics
        mutable std::mutex stats_mutex_;         // Protects statistics
        std::atomic<size_t> active_tasks_{0};    // Currently executing tasks
        std::atomic<size_t> completed_tasks_{0}; // Completed tasks
    };

    // Template implementation (must be in header for templates)
    template <typename F, typename... Args>
    auto ThreadPool::submit(F &&task, Args &&...args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {

        using return_type = typename std::invoke_result_t<F, Args...>;

        // Create a packaged_task that will execute our function
        auto packaged_task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(task), std::forward<Args>(args)...));

        // Get the future from the packaged_task
        std::future<return_type> result = packaged_task->get_future();

        {
            // Lock the queue to add the task
            std::lock_guard<std::mutex> lock(queue_mutex_);

            // Don't allow submission after shutdown
            if (stop_)
            {
                throw std::runtime_error("submit on stopped ThreadPool");
            }

            // Add the task to the queue
            tasks_.emplace([packaged_task]()
                           { (*packaged_task)(); });
        }

        // Notify one worker thread that a task is available
        condition_.notify_one();

        return result;
    }

} // namespace trading
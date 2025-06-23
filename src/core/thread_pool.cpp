#include "core/thread_pool.h"
#include <algorithm>
#include <stdexcept>

namespace trading
{

    ThreadPool::ThreadPool(size_t num_threads) : stop_(false)
    {
        // If no threads specified, use number of CPU cores
        if (num_threads == 0)
        {
            num_threads = std::thread::hardware_concurrency();
            if (num_threads == 0)
            {
                num_threads = 4; // Fallback to 4 threads
            }
        }

        // Create worker threads
        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers_.emplace_back(&ThreadPool::worker_function, this);
        }
    }

    ThreadPool::~ThreadPool()
    {
        shutdown();
    }

    ThreadPool::ThreadPool(ThreadPool &&other) noexcept
        : workers_(std::move(other.workers_)), tasks_(std::move(other.tasks_)), stop_(other.stop_.load()), active_tasks_(other.active_tasks_.load()), completed_tasks_(other.completed_tasks_.load())
    {
        other.stop_.store(true);
    }

    ThreadPool &ThreadPool::operator=(ThreadPool &&other) noexcept
    {
        if (this != &other)
        {
            shutdown();

            workers_ = std::move(other.workers_);
            tasks_ = std::move(other.tasks_);
            stop_ = other.stop_.load();
            active_tasks_ = other.active_tasks_.load();
            completed_tasks_ = other.completed_tasks_.load();

            other.stop_.store(true);
        }
        return *this;
    }

    size_t ThreadPool::pending_tasks() const
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

    void ThreadPool::wait_all()
    {
        // Wait until all tasks are completed
        while (true)
        {
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (tasks_.empty() && active_tasks_.load() == 0)
                {
                    break;
                }
            }
            std::this_thread::yield();
        }
    }

    void ThreadPool::shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }

        // Notify all waiting threads
        condition_.notify_all();

        // Wait for all threads to finish
        for (auto &worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }

        workers_.clear();
    }

    void ThreadPool::worker_function()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);

                // Wait for a task or shutdown signal
                condition_.wait(lock, [this]
                                { return stop_ || !tasks_.empty(); });

                // If shutting down and no tasks, exit
                if (stop_ && tasks_.empty())
                {
                    return;
                }

                // Get the next task
                if (!tasks_.empty())
                {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
            }

            // Execute the task
            if (task)
            {
                active_tasks_.fetch_add(1);

                try
                {
                    task();
                }
                catch (...)
                {
                    // Log error in production code
                    // For now, just continue
                }

                active_tasks_.fetch_sub(1);
                completed_tasks_.fetch_add(1);
            }
        }
    }

} // namespace trading
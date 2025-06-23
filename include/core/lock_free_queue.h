#pragma once

#include <atomic>
#include <memory>
#include <cstddef>

namespace trading
{

    /**
     * @brief Lock-free single-producer, single-consumer queue
     *
     * This class implements a lock-free queue that can be safely used between
     * one producer thread and one consumer thread without any locks. This is
     * essential for high-performance trading systems where lock contention
     * can cause significant performance degradation.
     *
     * Key features:
     * - Lock-free operation (no mutexes or condition variables)
     * - Single producer, single consumer design
     * - Fixed-size circular buffer
     * - Memory ordering guarantees
     * - Exception safety
     */
    template <typename T>
    class LockFreeQueue
    {
    public:
        /**
         * @brief Constructor
         * @param capacity Maximum number of elements the queue can hold
         *
         * Creates a lock-free queue with the specified capacity.
         * The actual capacity will be the next power of 2 greater than or equal to capacity.
         */
        explicit LockFreeQueue(size_t capacity);

        /**
         * @brief Destructor
         *
         * Cleans up allocated memory.
         */
        ~LockFreeQueue() = default;

        // Prevent copying
        LockFreeQueue(const LockFreeQueue &) = delete;
        LockFreeQueue &operator=(const LockFreeQueue &) = delete;

        // Allow moving
        LockFreeQueue(LockFreeQueue &&) noexcept = default;
        LockFreeQueue &operator=(LockFreeQueue &&) noexcept = default;

        /**
         * @brief Try to push an element to the queue
         * @param value Value to push
         * @return true if successful, false if queue is full
         *
         * Attempts to add an element to the queue. Returns false if the queue
         * is full. This method is thread-safe for the producer thread only.
         */
        bool try_push(const T &value);

        /**
         * @brief Try to push an element to the queue (move version)
         * @param value Value to push (will be moved)
         * @return true if successful, false if queue is full
         */
        bool try_push(T &&value);

        /**
         * @brief Try to pop an element from the queue
         * @param value Reference to store the popped value
         * @return true if successful, false if queue is empty
         *
         * Attempts to remove an element from the queue. Returns false if the
         * queue is empty. This method is thread-safe for the consumer thread only.
         */
        bool try_pop(T &value);

        /**
         * @brief Check if the queue is empty
         * @return true if queue is empty
         *
         * Note: This is a snapshot and may become stale immediately.
         */
        bool empty() const;

        /**
         * @brief Check if the queue is full
         * @return true if queue is full
         *
         * Note: This is a snapshot and may become stale immediately.
         */
        bool full() const;

        /**
         * @brief Get the current number of elements
         * @return Number of elements in the queue
         *
         * Note: This is a snapshot and may become stale immediately.
         */
        size_t size() const;

        /**
         * @brief Get the maximum capacity
         * @return Maximum number of elements the queue can hold
         */
        size_t capacity() const { return capacity_; }

    private:
        // Node structure for the queue
        struct Node
        {
            T data;
            std::atomic<size_t> sequence;

            Node() : sequence(0) {}
        };

        // Member variables
        std::unique_ptr<Node[]> buffer_; // Circular buffer
        size_t capacity_;                // Queue capacity (power of 2)
        size_t mask_;                    // Bit mask for modulo operation

        // Atomic indices
        std::atomic<size_t> head_; // Producer index
        std::atomic<size_t> tail_; // Consumer index

        // Helper methods
        static size_t next_power_of_2(size_t n);
        size_t increment(size_t index) const;
    };

    // Template implementation (must be in header for templates)

    template <typename T>
    LockFreeQueue<T>::LockFreeQueue(size_t capacity)
        : capacity_(next_power_of_2(capacity)), mask_(capacity_ - 1)
    {

        // Allocate buffer
        buffer_ = std::make_unique<Node[]>(capacity_);

        // Initialize sequence numbers
        for (size_t i = 0; i < capacity_; ++i)
        {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }

        // Initialize indices
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    template <typename T>
    bool LockFreeQueue<T>::try_push(const T &value)
    {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_acquire);

        // Check if queue is full
        if (head - tail >= capacity_)
        {
            return false;
        }

        // Get the node at head position
        Node &node = buffer_[head & mask_];
        size_t sequence = node.sequence.load(std::memory_order_acquire);

        // Check if this slot is available
        if (sequence != head)
        {
            return false;
        }

        // Try to increment head
        if (!head_.compare_exchange_weak(head, head + 1,
                                         std::memory_order_relaxed))
        {
            return false;
        }

        // Store the value
        node.data = value;

        // Update sequence number
        node.sequence.store(head + 1, std::memory_order_release);

        return true;
    }

    template <typename T>
    bool LockFreeQueue<T>::try_push(T &&value)
    {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_acquire);

        // Check if queue is full
        if (head - tail >= capacity_)
        {
            return false;
        }

        // Get the node at head position
        Node &node = buffer_[head & mask_];
        size_t sequence = node.sequence.load(std::memory_order_acquire);

        // Check if this slot is available
        if (sequence != head)
        {
            return false;
        }

        // Try to increment head
        if (!head_.compare_exchange_weak(head, head + 1,
                                         std::memory_order_relaxed))
        {
            return false;
        }

        // Store the value (move)
        node.data = std::move(value);

        // Update sequence number
        node.sequence.store(head + 1, std::memory_order_release);

        return true;
    }

    template <typename T>
    bool LockFreeQueue<T>::try_pop(T &value)
    {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t head = head_.load(std::memory_order_acquire);

        // Check if queue is empty
        if (tail >= head)
        {
            return false;
        }

        // Get the node at tail position
        Node &node = buffer_[tail & mask_];
        size_t sequence = node.sequence.load(std::memory_order_acquire);

        // Check if this slot contains data
        if (sequence != tail + 1)
        {
            return false;
        }

        // Try to increment tail
        if (!tail_.compare_exchange_weak(tail, tail + 1,
                                         std::memory_order_relaxed))
        {
            return false;
        }

        // Load the value
        value = std::move(node.data);

        // Update sequence number
        node.sequence.store(tail + capacity_, std::memory_order_release);

        return true;
    }

    template <typename T>
    bool LockFreeQueue<T>::empty() const
    {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t head = head_.load(std::memory_order_relaxed);
        return tail >= head;
    }

    template <typename T>
    bool LockFreeQueue<T>::full() const
    {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_relaxed);
        return head - tail >= capacity_;
    }

    template <typename T>
    size_t LockFreeQueue<T>::size() const
    {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_relaxed);
        return head - tail;
    }

    template <typename T>
    size_t LockFreeQueue<T>::next_power_of_2(size_t n)
    {
        if (n <= 1)
            return 1;

        size_t power = 1;
        while (power < n)
        {
            power <<= 1;
        }
        return power;
    }

    template <typename T>
    size_t LockFreeQueue<T>::increment(size_t index) const
    {
        return (index + 1) & mask_;
    }

} // namespace trading
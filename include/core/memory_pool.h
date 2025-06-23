#pragma once

#include <vector>
#include <mutex>
#include <memory>
#include <cstddef>
#include <atomic>

namespace trading
{

    /**
     * @brief Thread-safe memory pool for efficient allocation
     *
     * This class provides a pool of pre-allocated memory blocks that can be
     * quickly allocated and deallocated without system calls. This is crucial
     * for high-frequency trading where allocation overhead can impact performance.
     *
     * Key features:
     * - Thread-safe allocation/deallocation
     * - Fixed-size blocks for predictable performance
     * - Automatic expansion when pool is exhausted
     * - Memory reuse to reduce fragmentation
     */
    class MemoryPool
    {
    public:
        /**
         * @brief Constructor
         * @param block_size Size of each memory block in bytes
         * @param initial_blocks Number of blocks to pre-allocate
         *
         * Creates a memory pool with the specified block size and initial capacity.
         */
        explicit MemoryPool(size_t block_size, size_t initial_blocks = 100);

        /**
         * @brief Destructor
         *
         * Frees all allocated memory.
         */
        ~MemoryPool();

        // Prevent copying
        MemoryPool(const MemoryPool &) = delete;
        MemoryPool &operator=(const MemoryPool &) = delete;

        // Allow moving
        MemoryPool(MemoryPool &&) noexcept;
        MemoryPool &operator=(MemoryPool &&) noexcept;

        /**
         * @brief Allocate a memory block
         * @return Pointer to allocated memory block
         *
         * Returns a pointer to an available memory block. If no blocks are
         * available, the pool will automatically expand.
         */
        void *allocate();

        /**
         * @brief Deallocate a memory block
         * @param ptr Pointer to previously allocated block
         *
         * Returns a memory block to the pool for reuse.
         */
        void deallocate(void *ptr);

        /**
         * @brief Get the size of each block
         * @return Block size in bytes
         */
        size_t block_size() const { return block_size_; }

        /**
         * @brief Get the total number of blocks
         * @return Total blocks (allocated + free)
         */
        size_t total_blocks() const;

        /**
         * @brief Get the number of free blocks
         * @return Number of available blocks
         */
        size_t free_blocks() const;

        /**
         * @brief Get the number of allocated blocks
         * @return Number of currently allocated blocks
         */
        size_t allocated_blocks() const;

        /**
         * @brief Reserve additional blocks
         * @param num_blocks Number of blocks to add
         *
         * Pre-allocates additional memory blocks to avoid expansion during
         * critical operations.
         */
        void reserve(size_t num_blocks);

    private:
        // Memory block structure
        struct Block
        {
            Block *next;  // Next block in free list
            char data[1]; // Single byte array (we'll allocate more space)
        };

        // Member variables
        size_t block_size_;                           // Size of each block
        std::vector<std::unique_ptr<char[]>> chunks_; // Memory chunks
        Block *free_list_;                            // Linked list of free blocks

        // Synchronization
        mutable std::mutex pool_mutex_; // Protects pool state

        // Statistics
        size_t total_blocks_{0};                  // Total blocks created
        std::atomic<size_t> allocated_blocks_{0}; // Currently allocated blocks

        // Helper methods
        void expand_pool(size_t num_blocks);
        bool is_valid_pointer(void *ptr) const;
    };

    /**
     * @brief RAII wrapper for memory pool allocation
     *
     * This class provides automatic deallocation when it goes out of scope,
     * similar to std::unique_ptr but for memory pool allocations.
     */
    template <typename T>
    class PoolAllocator
    {
    public:
        explicit PoolAllocator(MemoryPool &pool) : pool_(pool) {}

        T *allocate(size_t n)
        {
            if (n != 1)
            {
                throw std::bad_alloc(); // Only support single object allocation
            }
            return static_cast<T *>(pool_.allocate());
        }

        void deallocate(T *ptr, size_t)
        {
            pool_.deallocate(ptr);
        }

        template <typename U, typename... Args>
        void construct(U *ptr, Args &&...args)
        {
            new (ptr) U(std::forward<Args>(args)...);
        }

        template <typename U>
        void destroy(U *ptr)
        {
            ptr->~U();
        }

    private:
        MemoryPool &pool_;
    };

} // namespace trading
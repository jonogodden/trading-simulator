#include "core/memory_pool.h"
#include <cassert>
#include <stdexcept>

namespace trading
{

    MemoryPool::MemoryPool(size_t block_size, size_t initial_blocks)
        : block_size_(block_size), free_list_(nullptr)
    {

        if (block_size == 0)
        {
            throw std::invalid_argument("Block size must be greater than 0");
        }

        // Add header size to block size
        size_t total_block_size = sizeof(Block) + block_size - 1; // -1 because data[1] already includes one byte

        // Ensure proper alignment
        if (total_block_size % alignof(Block) != 0)
        {
            total_block_size += alignof(Block) - (total_block_size % alignof(Block));
        }

        block_size_ = total_block_size;

        // Initialize the pool with initial blocks
        if (initial_blocks > 0)
        {
            expand_pool(initial_blocks);
        }
    }

    MemoryPool::~MemoryPool()
    {
        // All memory is automatically freed when chunks_ is destroyed
    }

    MemoryPool::MemoryPool(MemoryPool &&other) noexcept
        : block_size_(other.block_size_), chunks_(std::move(other.chunks_)), free_list_(other.free_list_), total_blocks_(other.total_blocks_), allocated_blocks_(other.allocated_blocks_.load())
    {

        other.free_list_ = nullptr;
        other.total_blocks_ = 0;
        other.allocated_blocks_.store(0);
    }

    MemoryPool &MemoryPool::operator=(MemoryPool &&other) noexcept
    {
        if (this != &other)
        {
            block_size_ = other.block_size_;
            chunks_ = std::move(other.chunks_);
            free_list_ = other.free_list_;
            total_blocks_ = other.total_blocks_;
            allocated_blocks_.store(other.allocated_blocks_.load());

            other.free_list_ = nullptr;
            other.total_blocks_ = 0;
            other.allocated_blocks_.store(0);
        }
        return *this;
    }

    void *MemoryPool::allocate()
    {
        std::lock_guard<std::mutex> lock(pool_mutex_);

        // If no free blocks, expand the pool
        if (free_list_ == nullptr)
        {
            expand_pool(std::max(size_t(1), total_blocks_ / 2));
        }

        // Get a block from the free list
        Block *block = free_list_;
        free_list_ = free_list_->next;

        // Update statistics
        allocated_blocks_.fetch_add(1);

        return block->data;
    }

    void MemoryPool::deallocate(void *ptr)
    {
        if (ptr == nullptr)
        {
            return;
        }

        // Validate the pointer
        if (!is_valid_pointer(ptr))
        {
            // In production, you might want to log this or throw an exception
            return;
        }

        std::lock_guard<std::mutex> lock(pool_mutex_);

        // Convert pointer back to block
        Block *block = reinterpret_cast<Block *>(
            reinterpret_cast<char *>(ptr) - offsetof(Block, data));

        // Add block back to free list
        block->next = free_list_;
        free_list_ = block;

        // Update statistics
        allocated_blocks_.fetch_sub(1);
    }

    size_t MemoryPool::total_blocks() const
    {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        return total_blocks_;
    }

    size_t MemoryPool::free_blocks() const
    {
        std::lock_guard<std::mutex> lock(pool_mutex_);

        size_t count = 0;
        Block *current = free_list_;
        while (current != nullptr)
        {
            ++count;
            current = current->next;
        }

        return count;
    }

    size_t MemoryPool::allocated_blocks() const
    {
        return allocated_blocks_.load();
    }

    void MemoryPool::reserve(size_t num_blocks)
    {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        expand_pool(num_blocks);
    }

    void MemoryPool::expand_pool(size_t num_blocks)
    {
        // Calculate chunk size
        size_t chunk_size = block_size_ * num_blocks;

        // Allocate new chunk
        auto chunk = std::make_unique<char[]>(chunk_size);

        // Initialize blocks in the chunk
        char *chunk_ptr = chunk.get();
        for (size_t i = 0; i < num_blocks; ++i)
        {
            Block *block = reinterpret_cast<Block *>(chunk_ptr);

            // Add to free list
            block->next = free_list_;
            free_list_ = block;

            chunk_ptr += block_size_;
        }

        // Store the chunk
        chunks_.push_back(std::move(chunk));
        total_blocks_ += num_blocks;
    }

    bool MemoryPool::is_valid_pointer(void *ptr) const
    {
        if (ptr == nullptr)
        {
            return false;
        }

        // Check if pointer is within any of our chunks
        for (const auto &chunk : chunks_)
        {
            char *chunk_start = chunk.get();
            char *chunk_end = chunk_start + (total_blocks_ * block_size_);
            char *ptr_char = static_cast<char *>(ptr);

            if (ptr_char >= chunk_start && ptr_char < chunk_end)
            {
                // Check alignment
                size_t offset = ptr_char - chunk_start;
                if (offset % block_size_ == offsetof(Block, data))
                {
                    return true;
                }
            }
        }

        return false;
    }

} // namespace trading
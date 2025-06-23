#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

// Include our core components
#include "core/thread_pool.h"
#include "core/memory_pool.h"
#include "core/lock_free_queue.h"

using namespace trading;

// Simple test function
void test_thread_pool_basic()
{
    std::cout << "Testing ThreadPool basic functionality..." << std::endl;

    ThreadPool pool(2);
    assert(pool.thread_count() == 2);

    auto future = pool.submit([]()
                              { return 42; });
    int result = future.get();
    assert(result == 42);

    std::cout << "ThreadPool basic test passed!" << std::endl;
}

void test_memory_pool_basic()
{
    std::cout << "Testing MemoryPool basic functionality..." << std::endl;

    MemoryPool pool(sizeof(int), 5);
    assert(pool.total_blocks() == 5);
    assert(pool.free_blocks() == 5);
    assert(pool.allocated_blocks() == 0);

    int *ptr1 = static_cast<int *>(pool.allocate());
    int *ptr2 = static_cast<int *>(pool.allocate());

    *ptr1 = 100;
    *ptr2 = 200;

    assert(*ptr1 == 100);
    assert(*ptr2 == 200);
    assert(pool.allocated_blocks() == 2);

    pool.deallocate(ptr1);
    pool.deallocate(ptr2);

    assert(pool.allocated_blocks() == 0);

    std::cout << "MemoryPool basic test passed!" << std::endl;
}

void test_lock_free_queue_basic()
{
    std::cout << "Testing LockFreeQueue basic functionality..." << std::endl;

    LockFreeQueue<int> queue(10);
    assert(queue.capacity() >= 10);
    assert(queue.empty());
    assert(!queue.full());

    // Test push and pop
    assert(queue.try_push(42));
    assert(!queue.empty());

    int value;
    assert(queue.try_pop(value));
    assert(value == 42);
    assert(queue.empty());

    std::cout << "LockFreeQueue basic test passed!" << std::endl;
}

int main()
{
    std::cout << "Running basic tests..." << std::endl;

    try
    {
        test_thread_pool_basic();
        test_memory_pool_basic();
        test_lock_free_queue_basic();

        std::cout << "All basic tests passed!" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
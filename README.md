# Trading Simulator - Phase 1

A high-performance multithreaded backtesting engine for algorithmic trading, built in C++.

## Project Overview

This is Phase 1 of a comprehensive trading simulator that will eventually include:
- Real-time market data simulation
- Monte Carlo option pricing
- Custom memory optimization
- Multithreaded strategy execution
- Advanced analytics and visualization

## Current Status (Phase 1)

We have implemented the foundational components:

### Core Infrastructure
- **ThreadPool**: High-performance thread pool for parallel task execution
- **MemoryPool**: Custom memory allocator for efficient memory management
- **LockFreeQueue**: Lock-free queue for high-performance inter-thread communication

### Key Features
- Modern C++17 with advanced features
- Thread-safe components
- Exception safety
- RAII resource management
- Comprehensive testing framework

## Project Structure

```
trading-simulator/
├── CMakeLists.txt              # Main build configuration
├── src/
│   ├── CMakeLists.txt          # Source build configuration
│   ├── main.cpp                # Main application entry point
│   ├── core/                   # Core infrastructure
│   │   ├── CMakeLists.txt
│   │   ├── thread_pool.cpp     # Thread pool implementation
│   │   ├── memory_pool.cpp     # Memory pool implementation
│   │   └── lock_free_queue.cpp # Lock-free queue implementation
│   ├── data/                   # Data processing (future)
│   ├── strategies/             # Trading strategies (future)
│   ├── analytics/              # Analytics engine (future)
│   └── visualization/          # Visualization (future)
├── include/                    # Header files
│   ├── core/
│   │   ├── thread_pool.h       # Thread pool interface
│   │   ├── memory_pool.h       # Memory pool interface
│   │   └── lock_free_queue.h   # Lock-free queue interface
│   ├── data/                   # Data headers (future)
│   ├── strategies/             # Strategy headers (future)
│   ├── analytics/              # Analytics headers (future)
│   └── visualization/          # Visualization headers (future)
├── tests/
│   ├── CMakeLists.txt          # Test build configuration
│   └── basic_tests.cpp         # Basic unit tests
├── data/                       # Data files (future)
├── external/                   # External dependencies (future)
└── README.md                   # This file
```

## Building the Project

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Make or Ninja build system

### Build Steps

1. **Create build directory:**
   ```bash
   mkdir build
   cd build
   ```

2. **Configure with CMake:**
   ```bash
   cmake ..
   ```

3. **Build the project:**
   ```bash
   cmake --build .
   ```

   Or on Unix-like systems:
   ```bash
   make
   ```

4. **Run the main application:**
   ```bash
   ./trading_simulator
   ```

5. **Run tests:**
   ```bash
   ./unit_tests
   ```

### CMake Explained

The build system uses CMake, which is a cross-platform build generator. Here's how it works:

1. **Main CMakeLists.txt**: Sets up the project, C++ standard, and includes subdirectories
2. **Subdirectory CMakeLists.txt**: Each component defines its own build rules
3. **Targets**: Each library/executable is a "target" that can be linked together
4. **Dependencies**: CMake automatically handles linking and include paths

## Component Details

### ThreadPool
- Manages a pool of worker threads
- Provides thread-safe task submission
- Supports futures for getting results
- Automatic load balancing
- Graceful shutdown

### MemoryPool
- Pre-allocates memory blocks
- Fast allocation/deallocation
- Thread-safe operations
- Automatic expansion when needed
- Reduces memory fragmentation

### LockFreeQueue
- Single-producer, single-consumer design
- No locks or mutexes
- High-performance inter-thread communication
- Fixed-size circular buffer
- Memory ordering guarantees

## Testing

The project includes basic unit tests to verify functionality:

- Thread pool task execution
- Memory pool allocation/deallocation
- Lock-free queue push/pop operations

Run tests with:
```bash
cd build
./unit_tests
```

## Performance Considerations

### ThreadPool
- Uses condition variables for efficient thread wakeup
- Lock-free task submission with futures
- Exception-safe task execution
- Configurable thread count

### MemoryPool
- O(1) allocation/deallocation
- No system calls during normal operation
- Automatic expansion with exponential growth
- Thread-safe with minimal locking

### LockFreeQueue
- Lock-free operations using atomic operations
- Single-producer, single-consumer for maximum performance
- Power-of-2 capacity for efficient modulo operations
- Memory ordering for correctness

## Future Phases

### Phase 2: Data Integration
- Yahoo Finance API integration
- Historical data processing
- Real-time data simulation
- Data caching and validation

### Phase 3: Trading Engine
- Strategy framework
- Portfolio management
- Order execution simulation
- Risk management

### Phase 4: Analytics
- Monte Carlo simulations
- Performance metrics
- Risk analysis
- Statistical modeling

### Phase 5: Visualization
- Chart generation
- Real-time dashboards
- Performance reporting
- Interactive analysis

## Learning Objectives

This project demonstrates:

1. **Modern C++**: Smart pointers, move semantics, lambdas, templates
2. **Multithreading**: Thread pools, atomic operations, lock-free programming
3. **Memory Management**: Custom allocators, RAII, resource management
4. **Build Systems**: CMake, dependency management, cross-platform builds
5. **Software Design**: Clean architecture, separation of concerns, testing

## Contributing

This is a learning project. Feel free to:
- Add new features
- Improve existing components
- Add more comprehensive tests
- Optimize performance
- Add documentation

## License

This project is for educational purposes. 
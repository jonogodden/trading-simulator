# src/CMakeLists.txt - Builds all source code components
# This file tells CMake how to build our main application

# Find required packages
# These are external libraries we'll need
find_package(Threads REQUIRED)  # For multithreading support

# Add subdirectories for each component
# Each subdirectory will have its own CMakeLists.txt
add_subdirectory(core)
add_subdirectory(data)
add_subdirectory(strategies)
add_subdirectory(analytics)
add_subdirectory(visualization)

# Create the main executable
# This is our trading simulator application
add_executable(trading_simulator
    main.cpp
)

# Link libraries
# We link against the libraries we created in subdirectories
target_link_libraries(trading_simulator
    core_lib
    data_lib
    strategies_lib
    analytics_lib
    visualization_lib
    Threads::Threads  # Link threading library
)

# Set include directories
# This tells the compiler where to find header files
target_include_directories(trading_simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Set compile definitions
# These are preprocessor definitions
target_compile_definitions(trading_simulator PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
) 
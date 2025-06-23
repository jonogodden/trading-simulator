# tests/CMakeLists.txt - Testing configuration
# This sets up our unit testing framework

# Enable testing
enable_testing()

# Find Google Test (we'll add this later)
# find_package(GTest REQUIRED)

# For now, we'll create a simple test executable
add_executable(unit_tests
    basic_tests.cpp
)

# Link against our libraries
target_link_libraries(unit_tests
    core_lib
    data_lib
    strategies_lib
    analytics_lib
    visualization_lib
)

# Set include directories
target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Add test
add_test(NAME UnitTests COMMAND unit_tests) 
# CMakeLists.txt - Main build configuration file
# This is the "root" CMake file that sets up the entire project

# Enable vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# CMake minimum version required
# We need 3.16+ for modern C++ features and better dependency management
cmake_minimum_required(VERSION 3.16)

# Project name and version
# This defines the project and sets up some basic variables
project(TradingSimulator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
# C++17 gives us many modern features like std::optional, std::variant, etc.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
# This allows us to use add_test() and ctest
enable_testing()

# Set build type if not specified
# Debug builds have more information for debugging
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler-specific settings
if(MSVC)
    # Microsoft Visual C++ specific settings
    add_compile_options(/W4 /utf-8)
else()
    # GCC/Clang specific settings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find external dependencies (optional for now)
find_package(CURL QUIET)

# Set flags for optional features
if(CURL_FOUND)
    add_compile_definitions(HAVE_CURL)
    message(STATUS "CURL found - HTTP functionality enabled")
else()
    message(STATUS "CURL not found - HTTP functionality disabled")
endif()

# Add subdirectories
# This tells CMake to look for more CMakeLists.txt files in these folders
add_subdirectory(src)
add_subdirectory(tests) 
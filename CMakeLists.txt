# CMakeLists.txt - Main build configuration file
# This is the "root" CMake file that sets up the entire project

# -----------------------------
# Optional vcpkg toolchain
# -----------------------------
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "vcpkg detected at $ENV{VCPKG_ROOT}")
else()
    message(WARNING "vcpkg not detected - continuing without external package manager")
endif()

# -----------------------------
# CMake minimum version required
# -----------------------------
cmake_minimum_required(VERSION 3.16)

# -----------------------------
# Project name and version
# -----------------------------
project(TradingSimulator VERSION 1.0.0 LANGUAGES CXX)

# -----------------------------
# Set C++ standard
# -----------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# Enable testing
# -----------------------------
enable_testing()

# -----------------------------
# Set build type if not specified
# -----------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# -----------------------------
# Compiler-specific settings
# -----------------------------
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -----------------------------
# Find external dependencies (CURL)
# -----------------------------
find_package(CURL QUIET)

if(CURL_FOUND)
    add_compile_definitions(HAVE_CURL)
    message(STATUS "CURL found - HTTP functionality enabled")
else()
    message(WARNING "CURL not found - HTTP functionality disabled")
endif()

# -----------------------------
# Add subdirectories
# -----------------------------
add_subdirectory(src)
add_subdirectory(tests)

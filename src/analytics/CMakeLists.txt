# src/analytics/CMakeLists.txt - Analytics library
# This builds our Monte Carlo and performance analysis components

# For now, create an empty library since we haven't implemented analytics components yet
add_library(analytics_lib INTERFACE)

target_include_directories(analytics_lib INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

# Link against core library for threading support
target_link_libraries(analytics_lib INTERFACE
    core_lib
) 
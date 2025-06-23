# src/strategies/CMakeLists.txt - Trading strategies library
# This builds our trading algorithm components

# For now, create an empty library since we haven't implemented strategy components yet
add_library(strategies_lib INTERFACE)

target_include_directories(strategies_lib INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

# Link against core and data libraries
target_link_libraries(strategies_lib INTERFACE
    core_lib
    data_lib
) 
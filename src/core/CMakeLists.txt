# src/core/CMakeLists.txt - Core library (thread pool, memory management)
# This builds our core infrastructure components

# Create a library for core components
# A library is reusable code that can be linked into other targets
add_library(core_lib
    thread_pool.cpp
    memory_pool.cpp
)

# Set include directories for this library
# PRIVATE means these includes are only for this library
target_include_directories(core_lib PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link against system libraries
# Threads::Threads provides threading support
target_link_libraries(core_lib
    Threads::Threads
)

# Set compile definitions
# These help with conditional compilation
target_compile_definitions(core_lib PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
) 
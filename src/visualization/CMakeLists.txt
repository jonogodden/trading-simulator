# src/visualization/CMakeLists.txt - Visualization library
# This builds our charting and dashboard components

# Create visualization library
add_library(visualization_lib STATIC
    chart_renderer.cpp
    dashboard.cpp
    data_export.cpp
)

target_include_directories(visualization_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link against required libraries
target_link_libraries(visualization_lib PUBLIC
    data_lib
    core_lib
)

# Set C++ standard
target_compile_features(visualization_lib PUBLIC cxx_std_17)

# Add compiler-specific flags
if(MSVC)
    target_compile_options(visualization_lib PRIVATE /W4)
else()
    target_compile_options(visualization_lib PRIVATE -Wall -Wextra -Wpedantic)
endif() 
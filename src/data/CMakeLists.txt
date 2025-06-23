# src/data/CMakeLists.txt - Data processing library
# This builds our data fetching and processing components

add_library(data_lib
    yahoo_finance.cpp
    data_processor.cpp
    cache_manager.cpp
)

target_include_directories(data_lib PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link against core library
target_link_libraries(data_lib
    core_lib
)

# Link optional external dependencies if found
if(CURL_FOUND)
    target_link_libraries(data_lib CURL::libcurl)
endif()

# nlohmann_json is header-only, no need to link 
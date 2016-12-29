cmake_minimum_required(VERSION 3.6)


set(SOURCE_FILES
        TrustLinesStorage.h
        TrustLinesStorage.cpp)

add_library(io__trust_lines_storage ${SOURCE_FILES})
target_link_libraries(io__trust_lines_storage
        common
        db__uuid_map_block_storage)
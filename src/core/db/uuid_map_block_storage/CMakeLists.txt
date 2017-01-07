cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Record.h
        Record.cpp
        UUIDMapBlockStorage.h
        UUIDMapBlockStorage.cpp)

add_library(db__uuid_map_block_storage ${SOURCE_FILES})
target_link_libraries(db__uuid_map_block_storage
        exceptions
        common)
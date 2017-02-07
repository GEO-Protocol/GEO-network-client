cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        MemoryUtils.h)

add_library(memory ${SOURCE_FILES})
target_link_libraries(memory
        common)
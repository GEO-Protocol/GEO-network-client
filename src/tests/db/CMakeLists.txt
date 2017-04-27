cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES)

add_library(db_tests ${SOURCE_FILES})
target_link_libraries(db_tests

        common
        exceptions)
cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        fields/tl_direction_column/TrustLineDirectionColumnTests.cpp
        fields/uuid_column/UUIDColumnTests.cpp)

add_library(db_tests ${SOURCE_FILES})
target_link_libraries(db_tests
        common
        exceptions
        db)
cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        OperationsLogTests.cpp RemoveOperationTests.cpp)

add_library(io_routing_tables_tests ${SOURCE_FILES})
target_link_libraries(io_routing_tables_tests
        io_routing_tables)
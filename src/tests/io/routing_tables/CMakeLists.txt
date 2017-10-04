cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        OperationsLogTests.cpp
        operations/RemoveOperationTests.cpp
        operations/DirectionUpdateOperationTests.cpp
        operations/SetOperationTests.cpp RoutingTableTests.cpp)

add_library(io_routing_tables_tests ${SOURCE_FILES})
target_link_libraries(io_routing_tables_tests
        common
        io_routing_tables)
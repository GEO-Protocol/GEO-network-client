cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        PropagationRoutingTablesTransaction.cpp
        PropagationRoutingTablesTransaction.h)

add_library(transactions__routing_tables ${SOURCE_FILES})
target_link_libraries(transactions__routing_tables
        transactions__base

        common
        time
        exceptions

        network__messages)

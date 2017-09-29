cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RoutingTableInitTransaction.h
        RoutingTableInitTransaction.cpp
        RoutingTableResponseTransaction.h
        RoutingTableResponseTransaction.cpp)

add_library(transactions__routing_table ${SOURCE_FILES})
target_link_libraries(transactions__routing_table
        transactions__base)
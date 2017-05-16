cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TrustLineStatesHandlerTransaction.h
        TrustLineStatesHandlerTransaction.cpp
        NeighborsCollectingTransaction.h
        NeighborsCollectingTransaction.cpp
        GetFirstRoutingTableTransaction.h
        GetFirstRoutingTableTransaction.cpp UpdateRoutingTablesTransaction.h UpdateRoutingTablesTransaction.cpp)

add_library(transactions__routing_tables ${SOURCE_FILES})
target_link_libraries(transactions__routing_tables
        transactions__base
        common
        exceptions)

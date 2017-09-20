cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RoutingTableRequestMessage.h
        RoutingTableRequestMessage.cpp RoutingTableResponseMessage.h RoutingTableResponseMessage.cpp)

add_library(messages__routing_table ${SOURCE_FILES})
target_link_libraries(messages__routing_table

        common
        exceptions
        #        time
        transactions)

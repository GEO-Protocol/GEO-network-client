cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RoutingTablesMessage.cpp
        RoutingTablesMessage.h)

add_library(messages__base__routing_tables ${SOURCE_FILES})
target_link_libraries(messages__base__routing_tables

        messages__base__abstract

        common
        exceptions)
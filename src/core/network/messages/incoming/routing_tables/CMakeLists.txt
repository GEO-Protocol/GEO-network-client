cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FirstLevelRoutingTableIncomingMessage.cpp
        FirstLevelRoutingTableIncomingMessage.h
        SecondLevelRoutingTableIncomingMessage.cpp
        SecondLevelRoutingTableIncomingMessage.h
        RoutingTableIncomingMessage.cpp
        RoutingTableIncomingMessage.h)

add_library(messages__incoming__routing_tables ${SOURCE_FILES})
target_link_libraries(messages__incoming__routing_tables

        messages__base__routing_tables

        common
        exceptions
        memory)

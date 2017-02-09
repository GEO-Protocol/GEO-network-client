cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FirstLevelRoutingTableIncomingMessage.cpp
        FirstLevelRoutingTableIncomingMessage.h
        RoutingTableIncomingMessage.cpp
        RoutingTableIncomingMessage.h
        SecondLevelRoutingTableIncomingMessage.cpp
        SecondLevelRoutingTableIncomingMessage.h)

add_library(messages__incoming__routing_tables ${SOURCE_FILES})
target_link_libraries(messages__incoming__routing_tables
        common
        exceptions)

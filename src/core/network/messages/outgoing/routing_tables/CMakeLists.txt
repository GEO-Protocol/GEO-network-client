cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FirstLevelRoutingTableOutgoingMessage.cpp
        FirstLevelRoutingTableOutgoingMessage.h
        RoutingTableOutgoingMessage.cpp
        RoutingTableOutgoingMessage.h
        SecondLevelRoutingTableOutgoingMessage.cpp
        SecondLevelRoutingTableOutgoingMessage.h)

add_library(messages__outgoing__routing_tables ${SOURCE_FILES})
target_link_libraries(messages__outgoing__routing_tables

        messages__base__routing_tables

        common
        exceptions
        memory)

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RoutingTableOutgoingMessage.cpp
        RoutingTableOutgoingMessage.h
        FirstLevelRoutingTableOutgoingMessage.cpp
        FirstLevelRoutingTableOutgoingMessage.h
        SecondLevelRoutingTableOutgoingMessage.cpp
        SecondLevelRoutingTableOutgoingMessage.h RoutingTableUpdateOutgoingMessage.cpp RoutingTableUpdateOutgoingMessage.h)

add_library(messages__outgoing__routing_tables ${SOURCE_FILES})
target_link_libraries(messages__outgoing__routing_tables

        messages__base__routing_tables

        common
        exceptions
        #        memory
        )
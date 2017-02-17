cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Response.h
        Response.cpp
        RoutingTablesResponse.cpp RoutingTablesResponse.h)

add_library(messages__response ${SOURCE_FILES})
target_link_libraries(messages__response
        common
        exceptions)

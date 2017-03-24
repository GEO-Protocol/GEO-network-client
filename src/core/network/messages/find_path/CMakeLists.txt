cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RequestRoutingTablesMessage.cpp
        RequestRoutingTablesMessage.h ResultRoutingTablesMessage.cpp ResultRoutingTablesMessage.h)

add_library(messages__find_path ${SOURCE_FILES})
target_link_libraries(messages__find_path

        common
        exceptions)
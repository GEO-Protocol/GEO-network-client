cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RequestRoutingTablesMessage.h
        RequestRoutingTablesMessage.cpp
        ResultRoutingTablesMessage.h
        ResultRoutingTablesMessage.cpp)

add_library(messages__find_path ${SOURCE_FILES})

target_link_libraries(messages__find_path)
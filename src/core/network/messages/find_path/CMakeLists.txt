cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RequestRoutingTablesMessage.h
        RequestRoutingTablesMessage.cpp
        ResultRoutingTablesMessage.h
        ResultRoutingTablesMessage.cpp
        ResultRoutingTable1LevelMessage.h
        ResultRoutingTable1LevelMessage.cpp
        ResultRoutingTable2LevelMessage.h
        ResultRoutingTable2LevelMessage.cpp
        ResultRoutingTable3LevelMessage.h
        ResultRoutingTable3LevelMessage.cpp)

add_library(messages__find_path ${SOURCE_FILES})

target_link_libraries(messages__find_path)
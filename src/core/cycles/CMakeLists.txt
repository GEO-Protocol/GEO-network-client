cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES

        CyclesManager.h
        CyclesManager.cpp

        RoutingTableManager.h
        RoutingTableManager.cpp)

add_library(cycles ${SOURCE_FILES})
target_link_libraries(cycles)
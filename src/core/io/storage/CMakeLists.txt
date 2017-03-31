cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        StorageHandler.h
        StorageHandler.cpp
        RoutingTableHandler.cpp
        RoutingTableHandler.h
        RoutingTablesHandler.cpp
        RoutingTablesHandler.h)

add_library(io__storage ${SOURCE_FILES})

target_link_libraries(io__storage
        libsqlite3)
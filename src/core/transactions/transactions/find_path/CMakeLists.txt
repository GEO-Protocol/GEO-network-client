cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FindPathByMaxFlowTransaction.h
        FindPathByMaxFlowTransaction.cpp)

add_library(transactions__find_path ${SOURCE_FILES})

target_link_libraries(transactions__find_path
        transactions__base
        resources_manager
        io__storage)
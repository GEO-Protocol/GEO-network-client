cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FindPathTransaction.cpp
        FindPathTransaction.h)

add_library(transactions__find_path ${SOURCE_FILES})

target_link_libraries(transactions__find_path
        ${Boost_SYSTEM_LIBRARY}
        paths
        transactions__base

        common
        time
        exceptions

        network__messages)
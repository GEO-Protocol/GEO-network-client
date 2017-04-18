cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TransactionsManager.cpp
        TransactionsManager.h)

add_library(transactions__manager ${SOURCE_FILES})
target_link_libraries(transactions__manager
        transactions__base

        common
        time
        exceptions

        settings
        network

        interface__commands
        interface__results
        trust_lines)

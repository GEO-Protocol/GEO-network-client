cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        NoEquivalentTransaction.cpp NoEquivalentTransaction.h)

add_library(transactions__no_equivalent ${SOURCE_FILES})

target_link_libraries(transactions__no_equivalent
        transactions__base
        messages__no_equivalent)
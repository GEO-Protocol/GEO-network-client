cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        NoEquivalentTransaction.h
        NoEquivalentTransaction.cpp

        PongReactionTransaction.h
        PongReactionTransaction.cpp)

add_library(transactions__general ${SOURCE_FILES})

target_link_libraries(transactions__general
        transactions__base)
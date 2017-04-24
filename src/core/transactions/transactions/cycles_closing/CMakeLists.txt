cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        CycleCloserInitiatorTransaction.h
        CycleCloserInitiatorTransaction.cpp)

add_library(transactions__cycles_closing ${SOURCE_FILES})
target_link_libraries(transactions__cycles_closing
        transactions__base
        network__messages

        paths)

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        state/TransactionState.cpp
        state/TransactionState.h
        TransactionResult.cpp
        TransactionResult.h)

add_library(transactions__result ${SOURCE_FILES})
target_link_libraries(transactions__result
        common
        #        time
        exceptions)

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES

    InitChannelTransaction.h
    InitChannelTransaction.cpp ConfirmChannelTransaction.cpp ConfirmChannelTransaction.h)

add_library(transactions__channels ${SOURCE_FILES})
target_link_libraries(transactions__channels
    transactions__base
    common
    exceptions)

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TransactionMessage.cpp
        TransactionMessage.h)

add_library(messages__base__transaction ${SOURCE_FILES})
target_link_libraries(messages__base__transaction

        messages__base__abstract

        common
        exceptions
        memory
        transactions)
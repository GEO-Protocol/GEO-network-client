cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TransactionMessage.cpp
        TransactionMessage.h

        ConfirmationMessage.h
        ConfirmationMessage.cpp

        DestinationMessage.h
        DestinationMessage.cpp)

add_library(messages__base__transaction ${SOURCE_FILES})
target_link_libraries(messages__base__transaction

        common
        exceptions)

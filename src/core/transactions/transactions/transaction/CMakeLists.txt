cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        PaymentTransactionByCommandUUIDTransaction.h
        PaymentTransactionByCommandUUIDTransaction.cpp)

add_library(transactions__transaction ${SOURCE_FILES})

target_link_libraries(transactions__transaction)
cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        CoordinatorPaymentTransaction.h
        CoordinatorPaymentTransaction.cpp
        ReceiverPaymentTransaction.cpp
        ReceiverPaymentTransaction.h)

add_library(transactions__payments ${SOURCE_FILES})
target_link_libraries(transactions__payments
        transactions__base

        common
        time
        exceptions
        paths

        network__messages)

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        base/BasePaymentTransaction.h
        base/BasePaymentTransaction.cpp

        CoordinatorPaymentTransaction.h
        CoordinatorPaymentTransaction.cpp

        ReceiverPaymentTransaction.h
        ReceiverPaymentTransaction.cpp

        IntermediateNodePaymentTransaction.h
        IntermediateNodePaymentTransaction.cpp)

add_library(transactions__payments ${SOURCE_FILES})
target_link_libraries(transactions__payments
        transactions__base
        network__messages

        paths)

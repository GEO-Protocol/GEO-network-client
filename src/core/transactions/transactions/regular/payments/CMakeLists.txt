cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        base/BasePaymentTransaction.h
        base/BasePaymentTransaction.cpp

        base/PathStats.h
        base/PathStats.cpp

        CoordinatorPaymentTransaction.h
        CoordinatorPaymentTransaction.cpp

        ReceiverPaymentTransaction.h
        ReceiverPaymentTransaction.cpp

        IntermediateNodePaymentTransaction.h
        IntermediateNodePaymentTransaction.cpp

        VotesStatusResponsePaymentTransaction.h
        VotesStatusResponsePaymentTransaction.cpp

        CycleCloserInitiatorTransaction.h
        CycleCloserInitiatorTransaction.cpp

        CycleCloserIntermediateNodeTransaction.h
        CycleCloserIntermediateNodeTransaction.cpp)

add_library(transactions__payments ${SOURCE_FILES})
target_link_libraries(transactions__payments
        transactions__base
        network__messages

        paths)

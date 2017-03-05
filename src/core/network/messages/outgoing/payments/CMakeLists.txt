cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ReceiverInitPaymentMessage.h
        ReceiverInitPaymentMessage.cpp
        ReceiverApprovePaymentMessage.cpp
        ReceiverApprovePaymentMessage.h
        ReserveBalanceRequestMessage.h
        ReserveBalanceRequestMessage.cpp
        IntermediateNodeReservationResponse.h
        IntermediateNodeReservationResponse.cpp)

add_library(messages__outgoing__payments ${SOURCE_FILES})
target_link_libraries(messages__outgoing__payments

        messages__base__transaction

        common
        exceptions
        memory
        multiprecision)

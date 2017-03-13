cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        base/RequestMessage.h
        base/RequestMessage.cpp

        base/ResponseMessage.h
        base/ResponseMessage.cpp

        ReceiverInitPaymentRequestMessage.h
        ReceiverInitPaymentRequestMessage.cpp

        ReceiverInitPaymentResponseMessage.h
        ReceiverInitPaymentResponseMessage.cpp

        CoordinatorReservationRequestMessage.h
        CoordinatorReservationRequestMessage.cpp

        CoordinatorReservationResponseMessage.h
        CoordinatorReservationResponseMessage.cpp

        IntermediateNodeReservationRequestMessage.h
        IntermediateNodeReservationRequestMessage.cpp

        IntermediateNodeReservationResponseMessage.h
        IntermediateNodeReservationResponseMessage.cpp)

add_library(messages__outgoing__payments ${SOURCE_FILES})
target_link_libraries(messages__outgoing__payments

        messages__base__transaction

        common
        exceptions
        memory
        multiprecision)

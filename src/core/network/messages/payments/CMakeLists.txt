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
        IntermediateNodeReservationResponseMessage.cpp

        ParticipantsVotesMessage.cpp
        ParticipantsVotesMessage.h

        FinalPathConfigurationMessage.cpp
        FinalPathConfigurationMessage.h

        TTLPolongationMessage.h
        TTLPolongationMessage.cpp)

add_library(messages__payments ${SOURCE_FILES})
target_link_libraries(messages__payments

        messages__base__transaction

        common
        exceptions)

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ReceiverInitPaymentMessage.h
        ReceiverInitPaymentMessage.cpp
        ReceiverApproveMessage.cpp
        ReceiverApproveMessage.h
        )

add_library(messages__outgoing__payments ${SOURCE_FILES})
target_link_libraries(messages__outgoing__payments
        common
        exceptions)

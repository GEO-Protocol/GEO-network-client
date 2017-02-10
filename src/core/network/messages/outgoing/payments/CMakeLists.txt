cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ReceiverInitPaymentMessage.h
        ReceiverInitPaymentMessage.cpp
        OperationStateMessage.cpp
        OperationStateMessage.h)

add_library(messages__outgoing__payments ${SOURCE_FILES})
target_link_libraries(messages__outgoing__payments
        common
        exceptions)

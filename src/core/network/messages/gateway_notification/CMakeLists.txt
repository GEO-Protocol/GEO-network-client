cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        GatewayNotificationMessage.h
        GatewayNotificationMessage.cpp

        GatewayNotificationOneEquivalentMessage.h
        GatewayNotificationOneEquivalentMessage.cpp)

add_library(messages__gateway_notification ${SOURCE_FILES})
target_link_libraries(messages__gateway_notification

        messages__base__transaction

        common
        exceptions)

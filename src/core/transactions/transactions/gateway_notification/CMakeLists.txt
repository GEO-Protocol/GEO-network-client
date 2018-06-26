cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        GatewayNotificationSenderTransaction.h
        GatewayNotificationSenderTransaction.cpp

        GatewayNotificationReceiverTransaction.h
        GatewayNotificationReceiverTransaction.cpp

        RoutingTableUpdatingTransaction.h
        RoutingTableUpdatingTransaction.cpp)

add_library(transactions__gateway_notification ${SOURCE_FILES})

target_link_libraries(transactions__gateway_notification)
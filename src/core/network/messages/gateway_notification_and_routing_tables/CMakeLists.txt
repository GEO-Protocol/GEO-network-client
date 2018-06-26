cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        GatewayNotificationMessage.h
        GatewayNotificationMessage.cpp

        RoutingTableResponseMessage.h
        RoutingTableResponseMessage.cpp)

add_library(messages__gateway_notification_and_routing_tables ${SOURCE_FILES})
target_link_libraries(messages__gateway_notification_and_routing_tables

        messages__base__transaction

        common
        exceptions)

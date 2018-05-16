cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TopologyCacheUpdateDelayedTask.h
        TopologyCacheUpdateDelayedTask.cpp

        GatewayNotificationAndRoutingTablesDelayedTask.h
        GatewayNotificationAndRoutingTablesDelayedTask.cpp)

add_library(delayed_tasks
        ${SOURCE_FILES})
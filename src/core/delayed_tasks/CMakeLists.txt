cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TopologyCacheUpdateDelayedTask.h
        TopologyCacheUpdateDelayedTask.cpp

        NotifyThatIAmIsGatewayDelayedTask.h
        NotifyThatIAmIsGatewayDelayedTask.cpp)

add_library(delayed_tasks
        ${SOURCE_FILES})
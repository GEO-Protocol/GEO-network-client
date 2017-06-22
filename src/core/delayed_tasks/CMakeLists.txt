cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Cycles.h
        Cycles.cpp
        MaxFlowCalculationCacheUpdateDelayedTask.h
        MaxFlowCalculationCacheUpdateDelayedTask.cpp
        )

add_library(delayed_tasks
        ${SOURCE_FILES})
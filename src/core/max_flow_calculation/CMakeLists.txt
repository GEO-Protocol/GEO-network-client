cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        MaxFlowCalculationTrustLine.cpp
        MaxFlowCalculationTrustLine.h
        manager/MaxFlowCalculationTrustLineManager.cpp
        manager/MaxFlowCalculationTrustLineManager.h
        cashe/MaxFlowCalculationCache.cpp
        cashe/MaxFlowCalculationCache.h
        cashe/MaxFlowCalculationCacheManager.cpp
        cashe/MaxFlowCalculationCacheManager.h
        manager/MaxFlowCalculationTrustLineWithPtr.cpp
        manager/MaxFlowCalculationTrustLineWithPtr.h)

add_library(max_flow_calculation ${SOURCE_FILES})
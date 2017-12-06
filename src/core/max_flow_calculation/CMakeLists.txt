cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        MaxFlowCalculationTrustLine.h
        MaxFlowCalculationTrustLine.cpp
        manager/MaxFlowCalculationTrustLineManager.h
        manager/MaxFlowCalculationTrustLineManager.cpp
        cashe/MaxFlowCalculationCache.h
        cashe/MaxFlowCalculationCache.cpp
        cashe/MaxFlowCalculationCacheManager.h
        cashe/MaxFlowCalculationCacheManager.cpp
        manager/MaxFlowCalculationTrustLineWithPtr.h
        manager/MaxFlowCalculationTrustLineWithPtr.cpp
        cashe/MaxFlowCalculationNodeCache.h
        cashe/MaxFlowCalculationNodeCache.cpp
        cashe/MaxFlowCalculationNodeCacheManager.h
        cashe/MaxFlowCalculationNodeCacheManager.cpp)

add_library(max_flow_calculation ${SOURCE_FILES})
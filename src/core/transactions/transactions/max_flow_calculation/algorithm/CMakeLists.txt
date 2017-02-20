cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        MaxFlowCalculationTrustLine.cpp
        MaxFlowCalculationTrustLine.h
        MaxFlowCalculationTrustLineManager.cpp
        MaxFlowCalculationTrustLineManager.h)

add_library(transactions__max_flow_calculation__algorithm ${SOURCE_FILES})
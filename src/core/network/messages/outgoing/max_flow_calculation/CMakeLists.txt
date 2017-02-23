cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        InitiateMaxFlowCalculationMessage.h
        InitiateMaxFlowCalculationMessage.cpp
        SendResultMaxFlowCalculationFromTargetMessage.h
        SendResultMaxFlowCalculationFromTargetMessage.cpp
        SendMaxFlowCalculationSourceFstLevelMessage.cpp
        SendMaxFlowCalculationSourceFstLevelMessage.h
        SendMaxFlowCalculationTargetFstLevelMessage.cpp
        SendMaxFlowCalculationTargetFstLevelMessage.h
        MaxFlowCalculationSourceFstLevelOutMessage.cpp
        MaxFlowCalculationSourceFstLevelOutMessage.h
        MaxFlowCalculationTargetFstLevelOutMessage.cpp
        MaxFlowCalculationTargetFstLevelOutMessage.h
        SendResultMaxFlowCalculationFromSourceMessage.cpp
        SendResultMaxFlowCalculationFromSourceMessage.h)

add_library(messages__outgoing__max_flow_calculation ${SOURCE_FILES})
target_link_libraries(messages__outgoing__max_flow_calculation

        messages__base__max_flow_calculation

        common
        exceptions)
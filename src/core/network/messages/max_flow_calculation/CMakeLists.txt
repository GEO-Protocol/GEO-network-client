cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        InitiateMaxFlowCalculationMessage.h
        InitiateMaxFlowCalculationMessage.cpp
        MaxFlowCalculationSourceFstLevelMessage.h
        MaxFlowCalculationSourceFstLevelMessage.cpp
        MaxFlowCalculationSourceSndLevelMessage.h
        MaxFlowCalculationSourceSndLevelMessage.cpp
        MaxFlowCalculationTargetFstLevelMessage.h
        MaxFlowCalculationTargetFstLevelMessage.cpp
        MaxFlowCalculationTargetSndLevelMessage.h
        MaxFlowCalculationTargetSndLevelMessage.cpp
        ResultMaxFlowCalculationMessage.h
        ResultMaxFlowCalculationMessage.cpp
        ResultMaxFlowCalculationGatewayMessage.h
        ResultMaxFlowCalculationGatewayMessage.cpp )

add_library(messages__max_flow_calculation ${SOURCE_FILES})

target_link_libraries(messages__max_flow_calculation
        messages__base__max_flow_calculation)
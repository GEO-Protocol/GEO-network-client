cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ReceiveMaxFlowCalculationOnTargetMessage.h
        ReceiveMaxFlowCalculationOnTargetMessage.cpp
        ResultMaxFlowCalculationFromTargetMessage.h
        ResultMaxFlowCalculationFromTargetMessage.cpp
        MaxFlowCalculationSourceFstLevelInMessage.cpp
        MaxFlowCalculationSourceFstLevelInMessage.h
        MaxFlowCalculationTargetFstLevelInMessage.cpp
        MaxFlowCalculationTargetFstLevelInMessage.h
        MaxFlowCalculationSourceSndLevelInMessage.cpp
        MaxFlowCalculationSourceSndLevelInMessage.h
        MaxFlowCalculationTargetSndLevelInMessage.cpp
        MaxFlowCalculationTargetSndLevelInMessage.h
        ResultMaxFlowCalculationFromSourceMessage.cpp
        ResultMaxFlowCalculationFromSourceMessage.h)

add_library(messages__incoming__max_flow_calculation ${SOURCE_FILES})
target_link_libraries(messages__incoming__max_flow_calculation
        common
        exceptions)
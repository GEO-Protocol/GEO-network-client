cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        InitiateMaxFlowCalculationTransaction.h
        InitiateMaxFlowCalculationTransaction.cpp
        ReceiveMaxFlowCalculationOnTargetTransaction.h
        ReceiveMaxFlowCalculationOnTargetTransaction.cpp
        MaxFlowCalculationSourceFstLevelTransaction.h
        MaxFlowCalculationSourceFstLevelTransaction.cpp
        MaxFlowCalculationTargetFstLevelTransaction.h
        MaxFlowCalculationTargetFstLevelTransaction.cpp
        MaxFlowCalculationSourceSndLevelTransaction.h
        MaxFlowCalculationSourceSndLevelTransaction.cpp
        MaxFlowCalculationTargetSndLevelTransaction.h
        MaxFlowCalculationTargetSndLevelTransaction.cpp
        ReceiveResultMaxFlowCalculationTransaction.h
        ReceiveResultMaxFlowCalculationTransaction.cpp)

add_library(transactions__max_flow_calculation ${SOURCE_FILES})

target_link_libraries(transactions__max_flow_calculation
        max_flow_calculation)
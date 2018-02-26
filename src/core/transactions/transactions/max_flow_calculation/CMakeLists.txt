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
        ReceiveResultMaxFlowCalculationTransaction.cpp
        CollectTopologyTransaction.h
        CollectTopologyTransaction.cpp
        MaxFlowCalculationStepTwoTransaction.h
        MaxFlowCalculationStepTwoTransaction.cpp
        MaxFlowCalculationFullyTransaction.h
        MaxFlowCalculationFullyTransaction.cpp)

add_library(transactions__max_flow_calculation ${SOURCE_FILES})

target_link_libraries(transactions__max_flow_calculation
        transactions__base
        topology
        messages__max_flow_calculation)
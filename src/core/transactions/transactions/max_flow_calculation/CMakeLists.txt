cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        InitiateMaxFlowCalculationTransaction.h
        InitiateMaxFlowCalculationTransaction.cpp
        MaxFlowCalculationTransaction.cpp
        MaxFlowCalculationTransaction.h
        ReceiveMaxFlowCalculationOnTargetTransaction.cpp
        ReceiveMaxFlowCalculationOnTargetTransaction.h
        ReceiveResultMaxFlowCalculationFromTargetTransaction.cpp
        ReceiveResultMaxFlowCalculationFromTargetTransaction.h
        MaxFlowCalculationSourceFstLevelTransaction.cpp
        MaxFlowCalculationSourceFstLevelTransaction.h
        MaxFlowCalculationTargetFstLevelTransaction.cpp
        MaxFlowCalculationTargetFstLevelTransaction.h
        MaxFlowCalculationSourceSndLevelTransaction.cpp
        MaxFlowCalculationSourceSndLevelTransaction.h
        MaxFlowCalculationTargetSndLevelTransaction.cpp
        MaxFlowCalculationTargetSndLevelTransaction.h
        ReceiveResultMaxFlowCalculationFromSourceTransaction.cpp
        ReceiveResultMaxFlowCalculationFromSourceTransaction.h)

add_library(transactions__max_flow_calculation ${SOURCE_FILES})

target_link_libraries(transactions__max_flow_calculation
        ${Boost_SYSTEM_LIBRARY}
        max_flow_calculation
        transactions__base

        common
        time
        exceptions

        network__messages)
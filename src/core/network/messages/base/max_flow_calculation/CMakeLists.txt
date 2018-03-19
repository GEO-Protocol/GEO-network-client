cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        MaxFlowCalculationMessage.cpp
        MaxFlowCalculationMessage.h

        MaxFlowCalculationConfirmationMessage.h
        MaxFlowCalculationConfirmationMessage.cpp)

add_library(messages__base__max_flow_calculation ${SOURCE_FILES})

target_link_libraries(messages__base__max_flow_calculation
        messages__base__transaction

        common
        exceptions)
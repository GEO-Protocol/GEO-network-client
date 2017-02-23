cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        MaxFlowCalculationResultMessage.cpp
        MaxFlowCalculationResultMessage.h)

add_library(messages__base__max_flow_calculation_result ${SOURCE_FILES})
target_link_libraries(messages__base__max_flow_calculation_result

        messages__base__transaction

        common
        memory
        exceptions)
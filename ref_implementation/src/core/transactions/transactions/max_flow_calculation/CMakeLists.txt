# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

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
        max_flow_calculation
        messages__max_flow_calculation)
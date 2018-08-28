# This file is part of GEO Protocol.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
#
# No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

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

# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        MaxFlowCalculationTrustLine.h
        MaxFlowCalculationTrustLine.cpp
        manager/MaxFlowCalculationTrustLineManager.h
        manager/MaxFlowCalculationTrustLineManager.cpp
        cashe/MaxFlowCalculationCache.h
        cashe/MaxFlowCalculationCache.cpp
        cashe/MaxFlowCalculationCacheManager.h
        cashe/MaxFlowCalculationCacheManager.cpp
        manager/MaxFlowCalculationTrustLineWithPtr.h
        manager/MaxFlowCalculationTrustLineWithPtr.cpp
        cashe/MaxFlowCalculationNodeCache.h
        cashe/MaxFlowCalculationNodeCache.cpp
        cashe/MaxFlowCalculationNodeCacheManager.h
        cashe/MaxFlowCalculationNodeCacheManager.cpp)

add_library(max_flow_calculation ${SOURCE_FILES})
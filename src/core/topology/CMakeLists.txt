cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TopologyTrustLine.h
        TopologyTrustLine.cpp
        manager/TopologyTrustLinesManager.h
        manager/TopologyTrustLinesManager.cpp
        cashe/TopologyCache.h
        cashe/TopologyCache.cpp
        cashe/TopologyCacheManager.h
        cashe/TopologyCacheManager.cpp
        manager/TopologyTrustLineWithPtr.h
        manager/TopologyTrustLineWithPtr.cpp
        cashe/MaxFlowCache.h
        cashe/MaxFlowCache.cpp
        cashe/MaxFlowCacheManager.h
        cashe/MaxFlowCacheManager.cpp)

add_library(topology ${SOURCE_FILES})
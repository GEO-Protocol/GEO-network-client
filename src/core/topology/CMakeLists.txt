cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TopologyTrustLine.h
        TopologyTrustLine.cpp
        manager/TopologyTrustLineManager.h
        manager/TopologyTrustLineManager.cpp
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
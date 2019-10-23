cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TopologyTrustLine.h
        TopologyTrustLine.cpp
        manager/TopologyTrustLineWithPtr.h
        manager/TopologyTrustLineWithPtr.cpp
        manager/TopologyTrustLinesManager.h
        manager/TopologyTrustLinesManager.cpp

        cache/TopologyCache.h
        cache/TopologyCache.cpp
        cache/TopologyCacheManager.h
        cache/TopologyCacheManager.cpp

        cache/MaxFlowCache.h
        cache/MaxFlowCache.cpp
        cache/MaxFlowCacheManager.h
        cache/MaxFlowCacheManager.cpp)

add_library(topology ${SOURCE_FILES})
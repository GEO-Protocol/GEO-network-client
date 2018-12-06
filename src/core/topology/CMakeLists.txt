cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TopologyTrustLine.h
        TopologyTrustLine.cpp
        TopologyTrustLineNew.h
        TopologyTrustLineNew.cpp
        manager/TopologyTrustLineWithPtr.h
        manager/TopologyTrustLineWithPtr.cpp
        manager/TopologyTrustLineWithPtrNew.h
        manager/TopologyTrustLineWithPtrNew.cpp
        manager/TopologyTrustLinesManager.h
        manager/TopologyTrustLinesManager.cpp

        cashe/TopologyCache.h
        cashe/TopologyCache.cpp
        cashe/TopologyCacheNew.h
        cashe/TopologyCacheNew.cpp
        cashe/TopologyCacheManager.h
        cashe/TopologyCacheManager.cpp

        cashe/MaxFlowCache.h
        cashe/MaxFlowCache.cpp
        cashe/MaxFlowCacheManager.h
        cashe/MaxFlowCacheManager.cpp)

add_library(topology ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        NodeUUID.cpp
        NodeUUID.h
        Types.h
        time/TimeUtils.h
        multiprecision/MultiprecisionUtils.h
        memory/MemoryUtils.h)

add_library(common ${SOURCE_FILES})
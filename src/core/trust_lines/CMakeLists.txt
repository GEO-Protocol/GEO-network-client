cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES

        manager/TrustLinesManager.cpp
        manager/TrustLinesManager.h

        TrustLine.cpp
        TrustLine.h)

add_library(trust_lines ${SOURCE_FILES})
target_link_libraries(trust_lines
        common
        reservations)
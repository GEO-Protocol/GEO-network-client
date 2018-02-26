cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TrustLinesManagersRouter.h
        TrustLinesManagersRouter.cpp)

add_library(equivalents ${SOURCE_FILES})
target_link_libraries(equivalents
        trust_lines)
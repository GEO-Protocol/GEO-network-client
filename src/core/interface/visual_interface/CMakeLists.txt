cmake_minimum_required(VERSION 3.6)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core/interface/)
set(SOURCE_FILES
        interface/VisualInterface.cpp
        interface/VisualInterface.h

        visual/VisualResult.cpp
        visual/VisualResult.h)

add_library(interface__visual ${SOURCE_FILES})
target_link_libraries(interface__visual
        common
        exceptions)

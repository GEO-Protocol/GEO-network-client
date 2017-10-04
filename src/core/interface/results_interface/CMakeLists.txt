cmake_minimum_required(VERSION 3.6)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core/interface/)
set(SOURCE_FILES
        interface/ResultsInterface.cpp
        interface/ResultsInterface.h

        result/CommandResult.cpp
        result/CommandResult.h)

add_library(interface__results ${SOURCE_FILES})
target_link_libraries(interface__results
        common
        #        time
        exceptions)

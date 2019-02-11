cmake_minimum_required(VERSION 3.6)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core/interface/)

set(SOURCE_FILES
        interface/EventsInterface.h
        interface/EventsInterface.cpp

        events/Event.h
        events/Event.cpp)

add_library(interface__events ${SOURCE_FILES})
target_link_libraries(interface__events
        common
        exceptions)

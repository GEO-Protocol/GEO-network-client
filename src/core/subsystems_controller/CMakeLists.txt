cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        SubsystemsController.h
        SubsystemsController.cpp
        TrustLinesInfluenceController.h
        TrustLinesInfluenceController.cpp)

add_library(subsystems_controller ${SOURCE_FILES})

target_link_libraries(subsystems_controller)
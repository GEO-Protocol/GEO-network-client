cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        EquivalentsSubsystemsRouter.h
        EquivalentsSubsystemsRouter.cpp

        EquivalentsCyclesSubsystemsRouter.h
        EquivalentsCyclesSubsystemsRouter.cpp)

add_library(equivalents ${SOURCE_FILES})
target_link_libraries(equivalents
        trust_lines
        topology
        cycles
        paths
        delayed_tasks)
cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        BoundaryNodeTopologyMessage.h
        BoundaryNodeTopologyMessage.cpp
        InBetweenNodeTopologyMessage.h
        InBetweenNodeTopologyMessage.cpp)

add_library(messages__cycles ${SOURCE_FILES})
target_link_libraries(messages__cycles

        messages__base__transaction

        common
        #        memory
        #        multiprecision
        exceptions)

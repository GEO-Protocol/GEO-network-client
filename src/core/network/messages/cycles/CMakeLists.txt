cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        BoundaryNodeTopologyMessage.h
        BoundaryNodeTopologyMessage.cpp
        InBetweenNodeTopologyMessage.h
        InBetweenNodeTopologyMessage.cpp
        ThreeNodes/ThreeNodesBalancesRequestMessage.h
        ThreeNodes/ThreeNodesBalancesRequestMessage.cpp
        ThreeNodes/ThreeNodesBalancesResponseMessage.h
        ThreeNodes/ThreeNodesBalancesResponseMessage.cpp
        FourNodes/FourNodesBalancesRequestMessage.h
        FourNodes/FourNodesBalancesRequestMessage.cpp
        FourNodes/FourNodesBalancesResponseMessage.h
        FourNodes/FourNodesBalancesResponseMessage.cpp
        )

add_library(messages__cycles ${SOURCE_FILES})
target_link_libraries(messages__cycles

        messages__base__transaction

        common
        memory
        multiprecision
        exceptions)

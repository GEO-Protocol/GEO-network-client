cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ThreeNodes/ThreeNodesBalancesRequestMessage.h
        ThreeNodes/ThreeNodesBalancesRequestMessage.cpp
        ThreeNodes/ThreeNodesBalancesResponseMessage.h
        ThreeNodes/ThreeNodesBalancesResponseMessage.cpp
        FourNodes/FourNodesBalancesRequestMessage.h
        FourNodes/FourNodesBalancesRequestMessage.cpp
        FourNodes/FourNodesBalancesResponseMessage.h
        FourNodes/FourNodesBalancesResponseMessage.cpp
        SixAndFiveNodes/base/CycleBaseFiveOrSixNodesInBetweenMessage.h
        SixAndFiveNodes/base/CycleBaseFiveOrSixNodesInBetweenMessage.cpp
        SixAndFiveNodes/base/CycleBaseFiveOrSixNodesBoundaryMessage.h
        SixAndFiveNodes/base/CycleBaseFiveOrSixNodesBoundaryMessage.cpp
        SixAndFiveNodes/CycleFiveNodesInBetweenMessage.hpp
        SixAndFiveNodes/CycleSixNodesInBetweenMessage.hpp
        SixAndFiveNodes/CycleFiveNodesBoundaryMessage.hpp
        SixAndFiveNodes/CycleSixNodesBoundaryMessage.hpp
        )

add_library(messages__cycles ${SOURCE_FILES})
target_link_libraries(messages__cycles

        messages__base__transaction

        common
        memory
        multiprecision
        exceptions)

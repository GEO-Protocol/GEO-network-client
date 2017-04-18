cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h
        ThreeNodes/CyclesThreeNodesBalancesRequestMessage.cpp
        ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h
        ThreeNodes/CyclesThreeNodesBalancesResponseMessage.cpp
        FourNodes/CyclesFourNodesBalancesRequestMessage.h
        FourNodes/CyclesFourNodesBalancesRequestMessage.cpp
        FourNodes/CyclesFourNodesBalancesResponseMessage.h
        FourNodes/CyclesFourNodesBalancesResponseMessage.cpp
        SixAndFiveNodes/base/CyclesBaseFiveOrSixNodesInBetweenMessage.h
        SixAndFiveNodes/base/CyclesBaseFiveOrSixNodesInBetweenMessage.cpp
        SixAndFiveNodes/base/CyclesBaseFiveOrSixNodesBoundaryMessage.h
        SixAndFiveNodes/base/CyclesBaseFiveOrSixNodesBoundaryMessage.cpp
        SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp
        SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp
        SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp
        SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp
        )

add_library(messages__cycles ${SOURCE_FILES})
target_link_libraries(messages__cycles

        messages__base__transaction

        common
        #        memory
        #        multiprecision
        exceptions)

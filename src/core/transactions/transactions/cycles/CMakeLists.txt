cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FiveAndSixNodes/base/CyclesBaseFiveSixNodesInitTransaction.cpp
        FiveAndSixNodes/base/CyclesBaseFiveSixNodesInitTransaction.h
        FiveAndSixNodes/CycleFiveNodesInitTransaction.cpp
        FiveAndSixNodes/CycleFiveNodesInitTransaction.h
        FiveAndSixNodes/CycleSixNodesInitTransaction.cpp
        FiveAndSixNodes/CycleSixNodesInitTransaction.h
        FiveAndSixNodes/CycleFiveNodesResponseTransaction.cpp
        FiveAndSixNodes/CycleFiveNodesResponseTransaction.h
        FiveAndSixNodes/CycleSixNodesResponseTransaction.h
        FiveAndSixNodes/CycleSixNodesResponseTransaction.cpp
        FourNodes/CycleFourNodesInitTransaction.h
        FourNodes/CycleFourNodesInitTransaction.cpp
        FourNodes/CycleFourNodesResponseTransaction.h
        FourNodes/CycleFourNodesResponseTransaction.cpp
        ThreeNodes/CycleThreeNodesInitTransaction.cpp
        ThreeNodes/CycleThreeNodesInitTransaction.h
        ThreeNodes/CycleThreeNodesResponseTransaction.cpp
        ThreeNodes/CycleThreeNodesResponseTransaction.h
        )

add_library(transactions__cycles ${SOURCE_FILES})
target_link_libraries(transactions__cycles
        transactions__base)

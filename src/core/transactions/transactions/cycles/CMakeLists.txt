cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FiveAndSixNodes/base/CyclesBaseFiveSixNodesInitTransaction.cpp
        FiveAndSixNodes/base/CyclesBaseFiveSixNodesInitTransaction.h
        FiveAndSixNodes/CyclesFiveNodesInitTransaction.cpp
        FiveAndSixNodes/CyclesFiveNodesInitTransaction.h
        FiveAndSixNodes/CyclesSixNodesInitTransaction.cpp
        FiveAndSixNodes/CyclesSixNodesInitTransaction.h
        FiveAndSixNodes/CyclesFiveNodesResponseTransaction.cpp
        FiveAndSixNodes/CyclesFiveNodesResponseTransaction.h
        FiveAndSixNodes/CyclesSixNodesResponseTransaction.h
        FiveAndSixNodes/CyclesSixNodesResponseTransaction.cpp
        FourNodes/CyclesFourNodesInitTransaction.h
        FourNodes/CyclesFourNodesInitTransaction.cpp
        FourNodes/CyclesFourNodesResponseTransaction.h
        FourNodes/CyclesFourNodesResponseTransaction.cpp
        ThreeNodes/CyclesThreeNodesInitTransaction.cpp
        ThreeNodes/CyclesThreeNodesInitTransaction.h
        ThreeNodes/CyclesThreeNodesResponseTransaction.cpp
        ThreeNodes/CyclesThreeNodesResponseTransaction.h
        )

add_library(transactions__cycles ${SOURCE_FILES})
target_link_libraries(transactions__cycles
        transactions__base)

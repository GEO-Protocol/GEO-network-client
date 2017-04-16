cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FiveAndSixNodes/base/CyclesBaseFiveSixNodesInitTransaction.cpp
        FiveAndSixNodes/base/CyclesBaseFiveSixNodesInitTransaction.h
        FiveAndSixNodes/CyclesFiveNodesInitTransaction.cpp
        FiveAndSixNodes/CyclesFiveNodesInitTransaction.h
        FiveAndSixNodes/CyclesSixNodesInitTransaction.cpp
        FiveAndSixNodes/CyclesSixNodesInitTransaction.h
        FiveAndSixNodes/CyclesFiveNodesReceiverTransaction.cpp
        FiveAndSixNodes/CyclesFiveNodesReceiverTransaction.h
        FiveAndSixNodes/CyclesSixNodesReceiverTransaction.h
        FiveAndSixNodes/CyclesSixNodesReceiverTransaction.cpp
        FourNodes/CyclesFourNodesInitTransaction.h
        FourNodes/CyclesFourNodesInitTransaction.cpp
        FourNodes/CyclesFourNodesReceiverTransaction.h
        FourNodes/CyclesFourNodesReceiverTransaction.cpp
        ThreeNodes/CyclesThreeNodesInitTransaction.cpp
        ThreeNodes/CyclesThreeNodesInitTransaction.h
        ThreeNodes/CyclesThreeNodesReceiverTransaction.cpp
        ThreeNodes/CyclesThreeNodesReceiverTransaction.h
        )

add_library(transactions__cycles ${SOURCE_FILES})
target_link_libraries(transactions__cycles
        transactions__base)

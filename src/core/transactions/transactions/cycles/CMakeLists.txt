cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ThreeNodes/ThreeNodesInitTransaction.cpp
        ThreeNodes/ThreeNodesInitTransaction.h
        ThreeNodes/ThreeNodesResponseTransaction.cpp
        ThreeNodes/ThreeNodesResponseTransaction.h
        )

add_library(transactions__cycles ${SOURCE_FILES})
target_link_libraries(transactions__cycles
        transactions__base)

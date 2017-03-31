cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ThreeNodes/GetThreeNodesNeighborBalancesTransaction.cpp
        ThreeNodes/GetThreeNodesNeighborBalancesTransaction.h
        FourNodes/GetFourNodesNeighborBalancesTransaction.h
        FourNodes/GetFourNodesNeighborBalancesTransaction.cpp
        GetTopologyAndBalancesTransaction.cpp
        GetTopologyAndBalancesTransaction.h
        FiveNodesTopologyTransaction.h
        FiveNodesTopologyTransaction.cpp
        SixNodesTopologyTransaction.h
        SixNodesTopologyTransaction.cpp
        )

add_library(transactions__cycles_old ${SOURCE_FILES})
target_link_libraries(transactions__cycles_old
        transactions__base

        common
        time
        exceptions

        network__messages)

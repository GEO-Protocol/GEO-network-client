cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        GetTopologyAndBalancesTransaction.cpp
        GetTopologyAndBalancesTransaction.h)

add_library(transactions__cycles ${SOURCE_FILES})
target_link_libraries(transactions__cycles
        transactions__base

        common
        #        time
        exceptions

        network__messages)

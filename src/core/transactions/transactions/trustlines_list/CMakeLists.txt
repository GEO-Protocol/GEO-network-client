cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        GetFirstLevelContractorsTransaction.h
        GetFirstLevelContractorsTransaction.cpp

        GetFirstLevelContractorsBalancesTransaction.h
        GetFirstLevelContractorsBalancesTransaction.cpp

        GetFirstLevelContractorBalanceTransaction.h
        GetFirstLevelContractorBalanceTransaction.cpp

        GetEquivalentListTransaction.h
        GetEquivalentListTransaction.cpp)

add_library(transactions__trustlines_list ${SOURCE_FILES})
target_link_libraries(transactions__trustlines_list
        transactions__base
        interface__commands)
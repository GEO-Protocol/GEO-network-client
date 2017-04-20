cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        GetFirstLevelContractorsTransaction.h
        GetFirstLevelContractorsTransaction.cpp

        GetFirstLevelContractorsBalancesTransaction.h
        GetFirstLevelContractorsBalancesTransaction.cpp
        )

add_library(transactions__contractors_list ${SOURCE_FILES})
target_link_libraries(transactions__contractors_list
        transactions__base)
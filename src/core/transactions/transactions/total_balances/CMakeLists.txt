cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TotalBalancesTransaction.h
        TotalBalancesTransaction.cpp)

add_library(transactions__total_balances ${SOURCE_FILES})

target_link_libraries(transactions__total_balances)
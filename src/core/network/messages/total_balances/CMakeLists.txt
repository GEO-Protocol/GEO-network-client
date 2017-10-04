cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        InitiateTotalBalancesMessage.h
        InitiateTotalBalancesMessage.cpp
        TotalBalancesResultMessage.h
        TotalBalancesResultMessage.cpp)

add_library(messages__total_balances ${SOURCE_FILES})

target_link_libraries(messages__total_balances)
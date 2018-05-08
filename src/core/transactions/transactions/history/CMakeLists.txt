cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        HistoryPaymentsTransaction.h
        HistoryPaymentsTransaction.cpp
        HistoryTrustLinesTransaction.h
        HistoryTrustLinesTransaction.cpp
        HistoryWithContractorTransaction.h
        HistoryWithContractorTransaction.cpp
        HistoryAdditionalPaymentsTransaction.h
        HistoryAdditionalPaymentsTransaction.cpp)

add_library(transactions__history ${SOURCE_FILES})

target_link_libraries(transactions__history)
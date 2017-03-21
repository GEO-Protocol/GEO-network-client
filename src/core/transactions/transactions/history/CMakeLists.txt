cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        HistoryPaymentsTransaction.cpp
        HistoryPaymentsTransaction.h HistoryTrustLinesTransaction.cpp HistoryTrustLinesTransaction.h)

add_library(transactions__history ${SOURCE_FILES})

target_link_libraries(transactions__history
        ${Boost_SYSTEM_LIBRARY}
        transactions__base
        db__operations_history_storage
        common
        time
        exceptions

        network__messages)
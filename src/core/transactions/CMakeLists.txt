cmake_minimum_required(VERSION 3.6)

add_subdirectory(manager)
add_subdirectory(scheduler)
add_subdirectory(transactions/result)
add_subdirectory(transactions/base)
add_subdirectory(transactions/regular/payments)
add_subdirectory(transactions/unique/trust_lines)
add_subdirectory(transactions/unique/routing_tables)
add_subdirectory(transactions/cycles)
add_subdirectory(transactions/max_flow_calculation)
add_subdirectory(transactions/total_balances)
add_subdirectory(transactions/history)
add_subdirectory(transactions/find_path)

set(SOURCE_FILES
        transactions/base/TransactionUUID.h transactions/contractors_list/GetFirstLevelContractorsTransaction.h transactions/contractors_list/GetFirstLevelContractorsTransaction.cpp)

add_library(transactions ${SOURCE_FILES})
target_link_libraries(transactions

        transactions__manager
        transactions__scheduler
        transactions__trustlines
        transactions__routing_tables
        transactions__max_flow_calculation
        transactions__payments
        transactions__cycles
        transactions__result
        transactions__total_balances
        transactions__history
        transactions__find_path

        common
        time
        memory
        exceptions)

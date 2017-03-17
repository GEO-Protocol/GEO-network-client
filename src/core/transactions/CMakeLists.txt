cmake_minimum_required(VERSION 3.6)

add_subdirectory(manager)
add_subdirectory(scheduler)
add_subdirectory(transactions/result)
add_subdirectory(transactions/base)
add_subdirectory(transactions/regular/payments)
add_subdirectory(transactions/unique/trust_lines)
add_subdirectory(transactions/unique/routing_tables)
add_subdirectory(transactions/unique/cycles)
add_subdirectory(transactions/max_flow_calculation)
add_subdirectory(transactions/total_balances)
add_subdirectory(transactions/history)

set(SOURCE_FILES
        transactions/base/TransactionUUID.h)

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

        common
        time
        memory
        exceptions)

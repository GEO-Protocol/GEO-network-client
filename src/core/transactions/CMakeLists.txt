cmake_minimum_required(VERSION 3.6)

#add_subdirectory(manager)
#add_subdirectory(scheduler)
add_subdirectory(transactions/trust_lines)
add_subdirectory(transactions/regular/payments)

add_subdirectory(transactions/result)
add_subdirectory(transactions/base)
add_subdirectory(transactions/cycles)
add_subdirectory(transactions/max_flow_calculation)
add_subdirectory(transactions/total_balances)
add_subdirectory(transactions/history)
add_subdirectory(transactions/find_path)
add_subdirectory(transactions/trustlines_list)
add_subdirectory(transactions/blacklist)
add_subdirectory(transactions/transaction)
add_subdirectory(transactions/gateway_notification)
add_subdirectory(transactions/no_equivalent)

set(SOURCE_FILES
        manager/TransactionsManager.h
        manager/TransactionsManager.cpp

        scheduler/TransactionsScheduler.h
        scheduler/TransactionsScheduler.cpp
        
        transactions/base/TransactionUUID.h)

add_library(transactions ${SOURCE_FILES})
target_link_libraries(transactions
        transactions__trustlines
        transactions__max_flow_calculation
        transactions__payments
        transactions__cycles
        transactions__result
        transactions__total_balances
        transactions__history
        transactions__trustlines_list
        transactions__find_path
        transactions__black_list
        transactions__transaction
        transactions__gateway_notification
        transactions__no_equivalent

        common
        exceptions
        equivalents)

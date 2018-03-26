# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

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
add_subdirectory(transactions/routing_table)
add_subdirectory(transactions/blacklist)
add_subdirectory(transactions/transaction)
add_subdirectory(transactions/gateway_notification)

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
        transactions__routing_table
        transactions__black_list
        transactions__transaction
        transactions__gateway_notification

        common
        exceptions)

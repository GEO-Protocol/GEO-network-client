cmake_minimum_required(VERSION 3.6)

add_subdirectory(manager)
add_subdirectory(scheduler)
add_subdirectory(transactions/result)
add_subdirectory(transactions/base)
add_subdirectory(transactions/regular/payments)
add_subdirectory(transactions/unique/trust_lines)
add_subdirectory(transactions/unique/routing_tables)

set(SOURCE_FILES
        )

add_library(transactions ${SOURCE_FILES})
target_link_libraries(transactions
        transactions__manager
        transactions__scheduler

        transactions__trustlines
        transactions__payments
        transactions__routing_tables
        transactions__result

        time
        common
        memory
        exceptions)

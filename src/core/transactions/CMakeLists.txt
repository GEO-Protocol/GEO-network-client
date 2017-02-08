cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TransactionUUID.h

        # todo: move this into separate library
        manager/TransactionsManager.cpp
        manager/TransactionsManager.h

        # todo: move this into separate library
        scheduler/TransactionsScheduler.cpp
        scheduler/TransactionsScheduler.h

        transactions/BaseTransaction.cpp
        transactions/BaseTransaction.h
        transactions/unique/UniqueTransaction.cpp
        transactions/unique/UniqueTransaction.h
        transactions/unique/trust_lines/TrustLineTransaction.cpp
        transactions/unique/trust_lines/TrustLineTransaction.h
        transactions/unique/trust_lines/OpenTrustLineTransaction.cpp
        transactions/unique/trust_lines/OpenTrustLineTransaction.h
        transactions/unique/trust_lines/AcceptTrustLineTransaction.cpp
        transactions/unique/trust_lines/AcceptTrustLineTransaction.h
        transactions/unique/trust_lines/CloseTrustLineTransaction.cpp
        transactions/unique/trust_lines/CloseTrustLineTransaction.h
        transactions/unique/trust_lines/RejectTrustLineTransaction.cpp
        transactions/unique/trust_lines/RejectTrustLineTransaction.h
        transactions/unique/trust_lines/SetTrustLineTransaction.cpp
        transactions/unique/trust_lines/SetTrustLineTransaction.h
        transactions/unique/trust_lines/UpdateTrustLineTransaction.cpp
        transactions/unique/trust_lines/UpdateTrustLineTransaction.h
        transactions/unique/routing_tables/SendRoutingTablesTransaction.cpp
        transactions/unique/routing_tables/SendRoutingTablesTransaction.h
        transactions/regular/payments/CoordinatorPaymentTransaction.cpp
        transactions/regular/payments/CoordinatorPaymentTransaction.h

        # todo: move this into separate library
        transactions/result/state/TransactionState.cpp
        transactions/result/state/TransactionState.h
        transactions/result/TransactionResult.cpp
        transactions/result/TransactionResult.h)

add_library(transactions ${SOURCE_FILES})
target_link_libraries(transactions
        common
        memory
        exceptions

        settings
        network

        interface__commands
        interface__results
        trust_lines)

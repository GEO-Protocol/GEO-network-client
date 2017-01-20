cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        manager/TransactionsManager.cpp
        manager/TransactionsManager.h

        scheduler/TransactionsScheduler.cpp
        scheduler/TransactionsScheduler.h

        transactions/BaseTransaction.cpp
        transactions/BaseTransaction.h

        transactions/unique/UniqueTransaction.cpp
        transactions/unique/UniqueTransaction.h
        transactions/unique/trust_lines/OpenTrustLineTransaction.cpp
        transactions/unique/trust_lines/OpenTrustLineTransaction.h
        transactions/unique/trust_lines/AcceptTrustLineTransaction.cpp
        transactions/unique/trust_lines/AcceptTrustLineTransaction.h

        transactions/result/state/TransactionState.cpp
        transactions/result/state/TransactionState.h
        transactions/result/TransactionResult.cpp
        transactions/result/TransactionResult.h

        TransactionUUID.cpp
        TransactionUUID.h transactions/unique/trust_lines/CloseTrustLineTransaction.cpp transactions/unique/trust_lines/CloseTrustLineTransaction.h)

add_library(transactions ${SOURCE_FILES})
target_link_libraries(transactions
        interface__commands
        interface__results
        common
        settings
        network
        trust_lines)
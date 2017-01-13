cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        manager/TransactionsManager.cpp
        manager/TransactionsManager.h
        scheduler/TransactionsScheduler.cpp
        scheduler/TransactionsScheduler.h
        transactions/BaseTransaction.cpp
        transactions/BaseTransaction.h
        transactions/CloseTrustLineTransaction.cpp
        transactions/CloseTrustLineTransaction.h
        transactions/ContractorsListTransaction.cpp
        transactions/ContractorsListTransaction.h
        transactions/MaximalAmountTransaction.cpp
        transactions/MaximalAmountTransaction.h
        transactions/OpenTrustLineTransaction.cpp
        transactions/OpenTrustLineTransaction.h
        transactions/state/TransactionState.cpp
        transactions/state/TransactionState.h
        transactions/TotalBalanceTransaction.cpp
        transactions/TotalBalanceTransaction.h
        transactions/UpdateTrustLineTransaction.cpp
        transactions/UpdateTrustLineTransaction.h
        transactions/UseCreditTransaction.cpp
        transactions/UseCreditTransaction.h
        TransactionUUID.cpp
        TransactionUUID.h
        observer/TransactionsObserver.cpp
        observer/TransactionsObserver.h
        transactions/UniqueTransaction.cpp
        transactions/UniqueTransaction.h
        )

add_library(transactions ${SOURCE_FILES})
target_link_libraries(transactions
        common
        network
        trust_lines)
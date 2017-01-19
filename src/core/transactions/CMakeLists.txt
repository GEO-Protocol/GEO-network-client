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
        transactions/unique/trust_lines/OpenTrustLineTransaction.cpp
        transactions/unique/trust_lines/OpenTrustLineTransaction.h
        transactions/result/state/TransactionState.cpp
        transactions/result/state/TransactionState.h
        transactions/TotalBalanceTransaction.cpp
        transactions/TotalBalanceTransaction.h
        transactions/UpdateTrustLineTransaction.cpp
        transactions/UpdateTrustLineTransaction.h
        transactions/unique/payment/UseCreditTransaction.cpp
        transactions/unique/payment/UseCreditTransaction.h
        TransactionUUID.cpp
        TransactionUUID.h
        transactions/unique/UniqueTransaction.cpp
        transactions/unique/UniqueTransaction.h
        transactions/unique/trust_lines/AcceptTrustLineTransaction.cpp
        transactions/unique/trust_lines/AcceptTrustLineTransaction.h
        transactions/result/TransactionResult.cpp
        transactions/result/TransactionResult.h)

add_library(transactions ${SOURCE_FILES})
target_link_libraries(transactions
        common
        settings
        network
        trust_lines)
cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        StorageHandler.h
        StorageHandler.cpp
        TrustLineHandler.h
        TrustLineHandler.cpp
        PaymentOperationStateHandler.h
        PaymentOperationStateHandler.cpp
        TransactionsHandler.h
        TransactionsHandler.cpp
        HistoryStorage.h
        HistoryStorage.cpp
        record/base/Record.h
        record/base/Record.cpp
        record/trust_line/TrustLineRecord.h
        record/trust_line/TrustLineRecord.cpp
        record/payment/PaymentRecord.h
        record/payment/PaymentRecord.cpp

        IOTransaction.cpp
        IOTransaction.h

        CommunicatorMessagesQueueHandler.h
        CommunicatorMessagesQueueHandler.cpp

        CommunicatorStorageHandler.h
        CommunicatorStorageHandler.cpp

        CommunicatorIOTransaction.h
        CommunicatorIOTransaction.cpp

        BlackListHandler.h
        BlackListHandler.cpp)

add_library(io__storage ${SOURCE_FILES})

target_link_libraries(io__storage
        libsqlite3)

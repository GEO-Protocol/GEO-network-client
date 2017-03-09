cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        record/base/Record.cpp
        record/base/Record.h
        record/trust_line/TrustLineRecord.cpp
        record/trust_line/TrustLineRecord.h
        record/payment/PaymentRecord.cpp
        record/payment/PaymentRecord.h
        storage/OperationsHistoryStorage.cpp
        storage/OperationsHistoryStorage.h)

add_library(db__operations_history_storage ${SOURCE_FILES})
target_link_libraries(db__operations_history_storage
        exceptions
        common)
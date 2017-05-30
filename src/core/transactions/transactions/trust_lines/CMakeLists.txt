cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES

        OpenTrustLineTransaction.cpp
        OpenTrustLineTransaction.h
        AcceptTrustLineTransaction.cpp
        AcceptTrustLineTransaction.h
        CloseTrustLineTransaction.cpp
        CloseTrustLineTransaction.h
        RejectTrustLineTransaction.cpp
        RejectTrustLineTransaction.h
        SetTrustLineTransaction.cpp
        SetTrustLineTransaction.h
        UpdateTrustLineTransaction.cpp
        UpdateTrustLineTransaction.h)

add_library(transactions__trustlines ${SOURCE_FILES})
target_link_libraries(transactions__trustlines
        transactions__base

        common
        #        time
        exceptions

        network__messages)

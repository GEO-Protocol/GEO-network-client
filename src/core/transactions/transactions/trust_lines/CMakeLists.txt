cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
    SetOutgoingTrustLineTransaction.h
    SetOutgoingTrustLineTransaction.cpp

    SetIncomingTrustLineTransaction.h
    SetIncomingTrustLineTransaction.cpp

    CloseIncomingTrustLineTransaction.h
    CloseIncomingTrustLineTransaction.cpp)

add_library(transactions__trustlines ${SOURCE_FILES})
target_link_libraries(transactions__trustlines
    transactions__base
    common
    exceptions
    network__messages)

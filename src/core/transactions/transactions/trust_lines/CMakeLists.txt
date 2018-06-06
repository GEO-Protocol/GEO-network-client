cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
    SetOutgoingTrustLineTransaction.h
    SetOutgoingTrustLineTransaction.cpp

    SetIncomingTrustLineTransaction.h
    SetIncomingTrustLineTransaction.cpp

    CloseIncomingTrustLineTransaction.h
    CloseIncomingTrustLineTransaction.cpp

    CloseOutgoingTrustLineTransaction.h
    CloseOutgoingTrustLineTransaction.cpp

    RejectOutgoingTrustLineTransaction.h
    RejectOutgoingTrustLineTransaction.cpp

    OpenTrustLineTransaction.h
    OpenTrustLineTransaction.cpp

    AcceptTrustLineTransaction.h
    AcceptTrustLineTransaction.cpp

    PublicKeysSharingSourceTransaction.h
    PublicKeysSharingSourceTransaction.cpp

    PublicKeysSharingTargetTransaction.h
    PublicKeysSharingTargetTransaction.cpp

    InitialAuditSourceTransaction.h
    InitialAuditSourceTransaction.cpp

    InitialAuditTargetTransaction.h
    InitialAuditTargetTransaction.cpp)

add_library(transactions__trustlines ${SOURCE_FILES})
target_link_libraries(transactions__trustlines
    transactions__base
    common
    exceptions
    network__messages)

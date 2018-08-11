cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        SetIncomingTrustLineMessage.h
        SetIncomingTrustLineMessage.cpp
        SetIncomingTrustLineInitialMessage.h
        SetIncomingTrustLineInitialMessage.cpp

        CloseOutgoingTrustLineMessage.h
        CloseOutgoingTrustLineMessage.cpp

        PublicKeyMessage.h
        PublicKeyMessage.cpp

        TrustLineConfirmationMessage.h
        TrustLineConfirmationMessage.cpp

        PublicKeyHashConfirmation.h
        PublicKeyHashConfirmation.cpp

        AuditMessage.h
        AuditMessage.cpp
        AuditResponseMessage.h
        AuditResponseMessage.cpp

        ConflictResolverMessage.h
        ConflictResolverMessage.cpp
        ConflictResolverResponseMessage.h
        ConflictResolverResponseMessage.cpp)

add_library(messages__trust_lines ${SOURCE_FILES})
target_link_libraries(messages__trust_lines
        common
        exceptions)


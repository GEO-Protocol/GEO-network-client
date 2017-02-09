cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        CloseTrustLineMessage.h
        CloseTrustLineMessage.cpp
        OpenTrustLineMessage.h
        OpenTrustLineMessage.cpp
        SetTrustLineMessage.h
        SetTrustLineMessage.cpp
        )

add_library(messages__outgoing__trust_lines ${SOURCE_FILES})
target_link_libraries(messages__outgoing__trust_lines
        common
        exceptions)

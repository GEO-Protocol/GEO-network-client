cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        SetIncomingTrustLineMessage.h
        SetIncomingTrustLineMessage.cpp

        CloseOutgoingTrustLineMessage.h
        CloseOutgoingTrustLineMessage.cpp)

add_library(messages__trust_lines ${SOURCE_FILES})
target_link_libraries(messages__trust_lines
        common
        exceptions)


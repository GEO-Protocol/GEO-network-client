cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TrustLinesMessage.cpp
        TrustLinesMessage.h )

add_library(messages__base__trust_lines ${SOURCE_FILES})
target_link_libraries(messages__base__trust_lines

        common
        exceptions
        memory
        transactions)
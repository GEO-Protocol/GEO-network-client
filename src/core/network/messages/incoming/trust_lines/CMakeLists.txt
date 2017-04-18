cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        AcceptTrustLineMessage.h
        AcceptTrustLineMessage.cpp
        RejectTrustLineMessage.h
        RejectTrustLineMessage.cpp
        UpdateTrustLineMessage.h
        UpdateTrustLineMessage.cpp)

add_library(messages__incoming__trust_lines ${SOURCE_FILES})
target_link_libraries(messages__incoming__trust_lines

        messages__base__trust_lines

        common
        exceptions
        #        memory
        #        multiprecision
        )


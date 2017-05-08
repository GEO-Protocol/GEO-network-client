cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        base/BaseTrustLineMessage.h
        base/BaseTrustLineMessage.cpp

        AcceptTrustLineMessage.h
        AcceptTrustLineMessage.cpp

        CloseTrustLineMessage.h
        CloseTrustLineMessage.cpp

        OpenTrustLineMessage.h
        OpenTrustLineMessage.cpp

        RejectTrustLineMessage.h
        RejectTrustLineMessage.cpp

        SetTrustLineMessage.h
        SetTrustLineMessage.cpp

        UpdateTrustLineMessage.h
        UpdateTrustLineMessage.cpp)

add_library(messages__trust_lines ${SOURCE_FILES})
target_link_libraries(messages__trust_lines
        common
        exceptions
        messages__result)


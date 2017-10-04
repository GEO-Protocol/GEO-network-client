cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        MessageResult.h
        MessageResult.cpp)

add_library(messages__result ${SOURCE_FILES})
target_link_libraries(messages__result

        common
        exceptions
        #        time
        transactions)

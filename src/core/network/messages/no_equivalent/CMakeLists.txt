cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        NoEquivalentMessage.cpp NoEquivalentMessage.h)

add_library(messages__no_equivalent ${SOURCE_FILES})
target_link_libraries(messages__no_equivalent
        common
        exceptions
        transactions)

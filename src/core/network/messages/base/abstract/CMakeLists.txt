cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        SenderMessage.cpp
        SenderMessage.h
        OtherMessage.cpp
        OtherMessage.h)

add_library(messages__base__abstract ${SOURCE_FILES})
target_link_libraries(messages__base__abstract

        common
        exceptions
        memory)
cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        PingMessage.h
        PingMessage.cpp
        PongMessage.h
        PongMessage.cpp)

add_library(messages__general ${SOURCE_FILES})

target_link_libraries(messages__general)
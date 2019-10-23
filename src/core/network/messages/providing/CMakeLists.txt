cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ProvidingAddressResponseMessage.h
        ProvidingAddressResponseMessage.cpp)

add_library(messages__providing ${SOURCE_FILES})
target_link_libraries(messages__providing
        common)


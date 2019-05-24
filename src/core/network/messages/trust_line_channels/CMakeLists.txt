cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        InitChannelMessage.h
        InitChannelMessage.cpp
        ConfirmChannelMessage.h
        ConfirmChannelMessage.cpp

        UpdateChannelAddressesMessage.h
        UpdateChannelAddressesMessage.cpp)

add_library(messages__trust_line_channels ${SOURCE_FILES})
target_link_libraries(messages__trust_line_channels
        common
        exceptions)


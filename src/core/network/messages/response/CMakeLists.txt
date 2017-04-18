cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Response.h
        Response.cpp
        )

add_library(messages__response ${SOURCE_FILES})
target_link_libraries(messages__response

        messages__base__transaction

        common
        exceptions
        #        memory
        )

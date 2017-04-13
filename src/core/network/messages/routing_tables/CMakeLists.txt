cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        )

add_library(messages__routing_tables ${SOURCE_FILES})
target_link_libraries(messages__routing_tables
        messages__base__transaction

        common
        exceptions
        memory)

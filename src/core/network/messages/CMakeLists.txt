cmake_minimum_required(VERSION 3.6)

add_subdirectory(base)
add_subdirectory(incoming)
add_subdirectory(outgoing)
add_subdirectory(response)
add_subdirectory(result)

set(SOURCE_FILES
        Message.hpp)

add_library(network__messages ${SOURCE_FILES})

target_link_libraries(network__messages
        messages__base__abstract
        messages__base__routing_tables
        messages__base__transaction
        messages__base__trust_lines

        messages__incoming__routing_tables
        messages__incoming__trust_lines

        messages__outgoing__payments
        messages__outgoing__trust_lines
        messages__outgoing__routing_tables

        messages__result
        messages__response)
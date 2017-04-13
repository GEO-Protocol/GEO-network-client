cmake_minimum_required(VERSION 3.6)

add_subdirectory(base)
add_subdirectory(routing_tables)

add_subdirectory(incoming) # todo: move all message from it up for one level
add_subdirectory(outgoing) # todo: move all message from it up for one level

add_subdirectory(response) # todo: there is no common responses in the system, this indicates invalid design
add_subdirectory(result)   # todo: there is no common responses in the system, this indicates invalid design
add_subdirectory(cycles)
add_subdirectory(max_flow_calculation)
add_subdirectory(total_balances)
add_subdirectory(find_path)

set(SOURCE_FILES
        SenderMessage.cpp   # todo: why are not this message in base?
        SenderMessage.h     # todo: why are not this message in base?
        Message.hpp)        # todo: why are not this message in base?


add_library(network__messages ${SOURCE_FILES})

target_link_libraries(network__messages
        messages__base__transaction
        messages__base__trust_lines
        messages__base__max_flow_calculation

        messages__cycles
        messages__max_flow_calculation


        messages__incoming__trust_lines

        messages__outgoing__payments
        messages__outgoing__trust_lines


        messages__result
        messages__response

        messages__total_balances

        messages__find_path)

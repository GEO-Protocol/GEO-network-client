cmake_minimum_required(VERSION 3.6)


add_subdirectory(base)

add_subdirectory(trust_lines)
add_subdirectory(payments)
add_subdirectory(cycles)
add_subdirectory(max_flow_calculation)
add_subdirectory(total_balances)
add_subdirectory(response) # todo: there is no common responses in the system, this indicates invalid design
add_subdirectory(result)   # todo: there is no common responses in the system, this indicates invalid design


set(SOURCE_FILES
        Message.hpp         # todo: why are not this message in base?
        SenderMessage.cpp   # todo: why are not this message in base?
        SenderMessage.h     # todo: why are not this message in base?

        debug/DebugMessage.h
        debug/DebugMessage.cpp)


add_library(network__messages ${SOURCE_FILES})

target_link_libraries(network__messages
        messages__base__transaction

        # todo: [MC] move base files into the cycles
        messages__base__max_flow_calculation

        messages__trust_lines
        messages__payments
        messages__cycles
        messages__max_flow_calculation
        messages__total_balances

        # todo: remove this
        messages__result
        messages__response)

cmake_minimum_required(VERSION 3.6)


add_subdirectory(base)

add_subdirectory(trust_lines)
add_subdirectory(payments)
add_subdirectory(cycles)
add_subdirectory(max_flow_calculation)
add_subdirectory(gateway_notification_and_routing_tables)
add_subdirectory(no_equivalent)


set(SOURCE_FILES
        Message.hpp         # todo: why are not this message in base?
        SenderMessage.cpp   # todo: why are not this message in base?
        SenderMessage.h     # todo: why are not this message in base?

        EquivalentMessage.h
        EquivalentMessage.cpp )


add_library(network__messages ${SOURCE_FILES})

target_link_libraries(network__messages
        messages__base__transaction

        # todo: [MC] move base files into the cycles
        messages__base__max_flow_calculation

        messages__trust_lines
        messages__payments
        messages__cycles
        messages__max_flow_calculation
        messages__gateway_notification_and_routing_tables
        messages__no_equivalent)

cmake_minimum_required(VERSION 3.6)

add_subdirectory(incoming)
add_subdirectory(outgoing)
add_subdirectory(response)
add_subdirectory(result)
add_subdirectory(cycles)

set(SOURCE_FILES
        Message.hpp
        TrustLinesMessage.hpp
        RoutingTablesMessage.hpp
        TransactionMessage.hpp
        MaxFlowCalculationMessage.hpp
        )

add_library(network__messages ${SOURCE_FILES})
target_link_libraries(network__messages
        messages__incoming__routing_tables
        messages__incoming__trust_lines
        messages__incoming__max_flow_calculation

        messages__outgoing__payments
        messages__outgoing__trust_lines
        messages__outgoing__routing_tables
        messages__outgoing__max_flow_calculation

        messages__result
        messages__response
        messages__cycles

        time
        )

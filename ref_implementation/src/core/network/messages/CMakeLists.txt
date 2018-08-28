# This file is part of GEO Protocol.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
#
# No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)


add_subdirectory(base)

add_subdirectory(trust_lines)
add_subdirectory(payments)
add_subdirectory(cycles)
add_subdirectory(max_flow_calculation)
add_subdirectory(routing_table)
add_subdirectory(gateway_notification)


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
        messages__routing_table
        messages__gateway_notification)

cmake_minimum_required(VERSION 3.6)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system filesystem REQUIRED)


set(SOURCE_FILES
        communicator/Communicator.cpp
        communicator/Communicator.h
        service/UUID2Address.cpp
        service/UUID2Address.h
        internal/IncomingMessagesHandler.cpp
        internal/IncomingMessagesHandler.h
        internal/OutgoingMessagesHandler.cpp
        internal/OutgoingMessagesHandler.h
        channels/packet/PacketHeader.cpp
        channels/packet/PacketHeader.h
        channels/packet/Packet.cpp
        channels/packet/Packet.h
        channels/channel/Channel.cpp
        channels/channel/Channel.h
        channels/manager/ChannelsManager.cpp
        channels/manager/ChannelsManager.h
        messages/Message.h
        messages/ServiceMessage.h
        messages/outgoing/trust_lines/OpenTrustLineMessage.cpp
        messages/outgoing/trust_lines/OpenTrustLineMessage.h
        messages/outgoing/trust_lines/CloseTrustLineMessage.cpp
        messages/outgoing/trust_lines/CloseTrustLineMessage.h
        messages/outgoing/trust_lines/SetTrustLineMessage.cpp
        messages/outgoing/trust_lines/SetTrustLineMessage.h
        messages/incoming/trust_lines/RejectTrustLineMessage.cpp
        messages/incoming/trust_lines/RejectTrustLineMessage.h
        messages/incoming/trust_lines/AcceptTrustLineMessage.cpp
        messages/incoming/trust_lines/AcceptTrustLineMessage.h
        messages/incoming/trust_lines/UpdateTrustLineMessage.cpp
        messages/incoming/trust_lines/UpdateTrustLineMessage.h
        messages/response/Response.cpp
        messages/response/Response.h
        messages/result/MessageResult.cpp
        messages/result/MessageResult.h messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.cpp messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.cpp messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h)

add_library(network ${SOURCE_FILES})
target_link_libraries(network
        ${Boost_SYSTEM_LIBRARY}

        common)
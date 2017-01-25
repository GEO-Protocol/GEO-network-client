cmake_minimum_required(VERSION 3.6)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system filesystem REQUIRED)


set(SOURCE_FILES
        communicator/Communicator.cpp
        communicator/Communicator.h
        internal/IncomingMessagesHandler.cpp
        internal/IncomingMessagesHandler.h
        internal/OutgoingMessagesHandler.cpp
        internal/OutgoingMessagesHandler.h
        messages/Message.h
        service/UUID2Address.cpp
        service/UUID2Address.h
        channels/channel/Channel.cpp
        channels/channel/Channel.h
        channels/packet/Packet.cpp
        channels/packet/Packet.h
        channels/manager/ChannelsManager.cpp
        channels/manager/ChannelsManager.h
        channels/packet/PacketHeader.cpp
        channels/packet/PacketHeader.h
        messages/outgoing/OpenTrustLineMessage.cpp
        messages/outgoing/OpenTrustLineMessage.h
        messages/incoming/AcceptTrustLineMessage.cpp
        messages/incoming/AcceptTrustLineMessage.h
        messages/result/MessageResult.cpp
        messages/result/MessageResult.h
        messages/response/Response.cpp
        messages/response/Response.h
        messages/outgoing/CloseTrustLineMessage.cpp
        messages/outgoing/CloseTrustLineMessage.h
        messages/incoming/RejectTrustLineMessage.cpp
        messages/incoming/RejectTrustLineMessage.h)

add_library(network ${SOURCE_FILES})
target_link_libraries(network
        ${Boost_SYSTEM_LIBRARY}

        common)
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
        messages/outgoing/OpenTrustLineMessage.cpp
        messages/outgoing/OpenTrustLineMessage.h
        messages/outgoing/CloseTrustLineMessage.cpp
        messages/outgoing/CloseTrustLineMessage.h
        messages/incoming/RejectTrustLineMessage.cpp
        messages/incoming/RejectTrustLineMessage.h
        messages/incoming/AcceptTrustLineMessage.cpp
        messages/incoming/AcceptTrustLineMessage.h
        messages/response/Response.cpp
        messages/response/Response.h
        messages/result/MessageResult.cpp
        messages/result/MessageResult.h messages/outgoing/SetTrustLineMessage.cpp messages/outgoing/SetTrustLineMessage.h messages/incoming/UpdateTrustLineMessage.cpp messages/incoming/UpdateTrustLineMessage.h)

add_library(network ${SOURCE_FILES})
target_link_libraries(network
        ${Boost_SYSTEM_LIBRARY}

        common)
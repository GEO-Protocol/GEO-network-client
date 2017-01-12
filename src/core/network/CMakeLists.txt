cmake_minimum_required(VERSION 3.6)

find_package(Boost COMPONENTS system filesystem REQUIRED)

set(SOURCE_FILES
        Communicator.cpp
        Communicator.h
        internal/IncomingMessagesHandler.cpp
        internal/IncomingMessagesHandler.h
        internal/OutgoingMessagesHandler.cpp
        internal/OutgoingMessagesHandler.h
        messages/Message.h
        UUID2Address.cpp
        UUID2Address.h
        channels/channel/Channel.cpp
        channels/channel/Channel.h
        channels/packet/Packet.cpp
        channels/packet/Packet.h
        channels/manager/ChannelsManager.cpp
        channels/manager/ChannelsManager.h
        channels/packet/PacketHeader.cpp
        channels/packet/PacketHeader.h
        messages/OpenTrustLineMessage.cpp
        messages/OpenTrustLineMessage.h)

add_library(network ${SOURCE_FILES})
target_link_libraries(network
        ${Boost_SYSTEM_LIBRARY}

        common)
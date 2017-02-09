cmake_minimum_required(VERSION 3.6)

set(Boost_USE_SHARED_LIBS ON)
find_package(Boost COMPONENTS system filesystem REQUIRED)

add_subdirectory(messages)


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
        )

add_library(network ${SOURCE_FILES})
target_link_libraries(network
        ${Boost_SYSTEM_LIBRARY}

        common
        exceptions
        network__messages)

cmake_minimum_required(VERSION 3.6)
find_package(Boost COMPONENTS system REQUIRED)


set(SOURCE_FILES
        Communicator.cpp
        Communicator.h

        internal/common/Types.h


        internal/incoming/IncomingMessagesHandler.h
        internal/incoming/IncomingMessagesHandler.cpp

        internal/incoming/IncomingRemoteNode.cpp
        internal/incoming/IncomingRemoteNode.h

        internal/incoming/IncomingNodesHandler.cpp
        internal/incoming/IncomingNodesHandler.h

        internal/incoming/IncomingChannel.cpp
        internal/incoming/IncomingChannel.h

        internal/incoming/MessageParser.h
        internal/incoming/MessageParser.cpp

        internal/packet/packet.h


        internal/outgoing/OutgoingMessagesHandler.h
        internal/outgoing/OutgoingMessagesHandler.cpp

        internal/outgoing/OutgoingNodesHandler.h
        internal/outgoing/OutgoingNodesHandler.cpp

        internal/outgoing/OutgoingRemoteNode.h
        internal/outgoing/OutgoingRemoteNode.cpp


        internal/uuid2address/UUID2Address.h
        internal/uuid2address/UUID2Address.cpp


        )


add_library(network__communicator ${SOURCE_FILES})
target_link_libraries(network__communicator
        ${Boost_SYSTEM_LIBRARY}

        common
        exceptions
        network__messages)

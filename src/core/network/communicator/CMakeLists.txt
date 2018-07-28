cmake_minimum_required(VERSION 3.6)
find_package(Boost COMPONENTS system REQUIRED)


set(SOURCE_FILES
        Communicator.cpp
        Communicator.h

        internal/common/Types.h
        internal/common/Packet.hpp

        # Incoming
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

        # Outgoing
        internal/outgoing/OutgoingMessagesHandler.h
        internal/outgoing/OutgoingMessagesHandler.cpp

        internal/outgoing/OutgoingNodesHandler.h
        internal/outgoing/OutgoingNodesHandler.cpp

        internal/outgoing/OutgoingRemoteNode.h
        internal/outgoing/OutgoingRemoteNode.cpp

        # uuid2address
        internal/uuid2address/UUID2Address.h
        internal/uuid2address/UUID2Address.cpp


        # Confirmation required messages queue
        internal/queue/ConfirmationRequiredMessagesQueue.h
        internal/queue/ConfirmationRequiredMessagesQueue.cpp
        internal/queue/ConfirmationRequiredMessagesHandler.h
        internal/queue/ConfirmationRequiredMessagesHandler.cpp

        internal/queue/ConfirmationNotStronglyRequiredMessagesHandler.h
        internal/queue/ConfirmationNotStronglyRequiredMessagesHandler.cpp
        internal/queue/ConfirmationNotStronglyRequiredMessagesQueue.h
        internal/queue/ConfirmationNotStronglyRequiredMessagesQueue.cpp

        internal/queue/ConfirmationResponseMessagesHandler.h
        internal/queue/ConfirmationResponseMessagesHandler.cpp
        internal/queue/ConfirmationCachedResponseMessage.h
        internal/queue/ConfirmationCachedResponseMessage.cpp)


add_library(network__communicator ${SOURCE_FILES})
target_link_libraries(network__communicator
        ${Boost_SYSTEM_LIBRARY}

        common
        exceptions
        logger
        network__messages)

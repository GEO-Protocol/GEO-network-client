# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

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
        internal/queue/ConfirmationRequiredMessagesHandler.cpp)


add_library(network__communicator ${SOURCE_FILES})
target_link_libraries(network__communicator
        ${Boost_SYSTEM_LIBRARY}

        common
        exceptions
        logger
        network__messages)

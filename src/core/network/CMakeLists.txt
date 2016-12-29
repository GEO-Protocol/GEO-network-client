cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Communicator.cpp
        Communicator.h
        internal/IncomingMessagesHandler.cpp
        internal/IncomingMessagesHandler.h
        internal/OutgoingMessagesHandler.cpp
        internal/OutgoingMessagesHandler.h
        internal/OutgoingMessagesQueue.cpp
        internal/OutgoingMessagesQueue.h
        messages/TestMessage.h
        messages/Message.h
        messages/ProcessingReportMessage.h
        messages/Message.cpp)
add_library(network ${SOURCE_FILES})
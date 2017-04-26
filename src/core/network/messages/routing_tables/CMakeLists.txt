cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        NotificationTrustLineRemovedMessage.cpp NotificationTrustLineRemovedMessage.h NeighborsRequestMessage.cpp NeighborsRequestMessage.h NeighborsResponseMessage.cpp NeighborsResponseMessage.h NotificationTrustLineCreatedMessage.cpp NotificationTrustLineCreatedMessage.h)

add_library(messages__routing_tables ${SOURCE_FILES})
target_link_libraries(messages__routing_tables
        messages__base__transaction

        common
        exceptions)

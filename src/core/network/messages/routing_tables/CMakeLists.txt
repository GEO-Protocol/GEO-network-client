cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        NotificationTrustLineRemovedMessage.cpp
        NotificationTrustLineRemovedMessage.h

        NeighborsRequestMessage.cpp
        NeighborsRequestMessage.h

        NeighborsResponseMessage.cpp
        NeighborsResponseMessage.h

        NotificationTrustLineCreatedMessage.cpp
        NotificationTrustLineCreatedMessage.h

        CRC32Rt2RequestMessage.h
        CRC32Rt2ResponseMessage.h
        CRC32Rt2ResponseMessage.cpp CRC32Rt2RequestMessage.cpp CRC32ThirdLevelResponseMessage.h CRC32ThirdLevelResponseMessage.cpp)

add_library(messages__routing_tables ${SOURCE_FILES})
target_link_libraries(messages__routing_tables
        messages__base__transaction

        common
        exceptions)

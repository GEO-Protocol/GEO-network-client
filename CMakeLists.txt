cmake_minimum_required(VERSION 3.6)
project(geo_network_client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp src/core/network/Communicator.cpp src/core/network/Communicator.h src/core/network/internal/OutgoingMessagesHandler.cpp src/core/network/internal/OutgoingMessagesHandler.h src/core/common/NodeUUID.cpp src/core/common/NodeUUID.h src/core/network/messages/Message.cpp src/core/network/messages/Message.h src/core/network/internal/OutgoingMessagesQueue.cpp src/core/network/internal/OutgoingMessagesQueue.h src/core/Core.cpp src/core/Core.h src/core/common/exceptions/Exception.cpp src/core/common/exceptions/Exception.h src/core/network/messages/TestMessage.h src/core/common/exceptions/IOError.cpp src/core/common/exceptions/IOError.h src/core/common/exceptions/ValueError.cpp src/core/common/exceptions/ValueError.h src/core/common/exceptions/MemoryError.h src/tests/network/FileBackedMessagesQueueTests.cpp)
add_executable(geo_network_client ${SOURCE_FILES})

find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(geo_network_client
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)
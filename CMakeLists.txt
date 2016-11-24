cmake_minimum_required(VERSION 3.6)
project(geo_network_client)

# NOTE: Do not forget to change it in release version
add_definitions(-DDEBUG -DTESTS)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp src/core/network/Communicator.cpp src/core/network/Communicator.h src/core/network/internal/OutgoingMessagesHandler.cpp src/core/network/internal/OutgoingMessagesHandler.h src/core/common/NodeUUID.cpp src/core/common/NodeUUID.h src/core/network/messages/Message.cpp src/core/network/messages/Message.h src/core/network/internal/OutgoingMessagesQueue.cpp src/core/network/internal/OutgoingMessagesQueue.h src/core/Core.cpp src/core/Core.h src/core/common/exceptions/Exception.cpp src/core/common/exceptions/Exception.h src/core/network/messages/TestMessage.h src/core/common/exceptions/IOError.h src/core/common/exceptions/ValueError.h src/core/common/exceptions/MemoryError.h src/core/common/exceptions/PreconditionFaultError.h src/tests/network/FileBackedMessagesQueueTests.cpp src/core/network/internal/IncomingMessagesHandler.cpp src/core/network/internal/IncomingMessagesHandler.h src/core/common/exceptions/ConflictError.h src/core/network/messages/ProcessingReportMessage.h src/core/settings/Settings.cpp src/core/settings/Settings.h src/libs/json.h src/core/commands/CommandsAPI.cpp src/core/commands/CommandsAPI.h src/core/interface/commands/CommandsInterface.cpp src/core/interface/commands/CommandsInterface.h src/core/interface/BaseFIFOInterface.h src/core/commands/Command.cpp src/core/commands/Command.h src/core/logger/Logger.h src/core/trust_lines/TrustLine.h src/core/trust_lines/TrustLine.cpp src/core/trust_lines/TrustLinesManager.h src/core/trust_lines/TrustLinesManager.cpp src/tests/interface/CommandsParserTests.cpp src/tests/trust_lines/TrustLinesManagerTest.h src/tests/trust_lines/TrustLinesManagerTest.cpp src/core/interface/results/ResultsInterface.h src/core/interface/results/ResultsInterface.cpp src/core/results/Result.h src/core/results/Result.cpp src/core/results/OpenTrustLineResult.h src/core/results/OpenTrustLineResult.cpp src/core/results/UpdateTrustLineResult.h src/core/results/UpdateTrustLineResult.cpp src/core/results/PaymentResult.h src/core/results/PaymentResult.cpp src/core/results/MaximalTransactionAmountResult.h src/core/results/MaximalTransactionAmountResult.cpp src/core/results/CloseTrustLineResult.h src/core/results/CloseTrustLineResult.cpp)
add_executable(geo_network_client ${SOURCE_FILES})

find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(geo_network_client
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)
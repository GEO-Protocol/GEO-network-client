cmake_minimum_required(VERSION 3.6)

find_package(Boost COMPONENTS system REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core/interface/)
set(SOURCE_FILES
        commands/BaseUserCommand.cpp
        commands/BaseUserCommand.h
        commands/CloseTrustLineCommand.cpp
        commands/CloseTrustLineCommand.h
        commands/ContractorsListCommand.cpp
        commands/ContractorsListCommand.h
        commands/MaximalTransactionAmountCommand.cpp
        commands/MaximalTransactionAmountCommand.h
        commands/OpenTrustLineCommand.cpp
        commands/OpenTrustLineCommand.h
        commands/TotalBalanceCommand.cpp
        commands/TotalBalanceCommand.h
        commands/UpdateTrustLineCommand.cpp
        commands/UpdateTrustLineCommand.h
        commands/UseCreditCommand.cpp
        commands/UseCreditCommand.h
        CommandUUID.h

        interface/CommandsInterface.cpp
        interface/CommandsInterface.h)

add_library(interface__commands ${SOURCE_FILES})
target_link_libraries(interface__commands
        ${Boost_SYSTEM_LIBRARY}

        common)
cmake_minimum_required(VERSION 3.6)

find_package(Boost COMPONENTS system REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core/interface/)
set(SOURCE_FILES
        commands/BaseUserCommand.cpp
        commands/BaseUserCommand.h
        commands/trust_lines/OpenTrustLineCommand.cpp
        commands/trust_lines/OpenTrustLineCommand.h
        CommandUUID.h

        interface/CommandsInterface.cpp
        interface/CommandsInterface.h)

add_library(interface__commands ${SOURCE_FILES})
target_link_libraries(interface__commands
        ${Boost_SYSTEM_LIBRARY}

        common)
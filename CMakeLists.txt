cmake_minimum_required(VERSION 3.6)
project(geo_network_client)

# NOTE: Do not forget to change it in release version
add_definitions(-DDEBUG -DTESTS -DINTERNAL_ARGUMENTS_VALIDATION)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src/core/common/)
add_subdirectory(src/core/common/exceptions)

add_subdirectory(src/core/db)
add_subdirectory(src/tests/db)

add_subdirectory(src/core/io/routing_tables)
add_subdirectory(src/tests/io/routing_tables)


add_subdirectory(src/core/network)

set(SOURCE_FILES
        main.cpp

        src/core/logger/Logger.h

        src/core/Core.cpp
        src/core/Core.h

        src/libs/json.h
        src/core/settings/Settings.cpp
        src/core/settings/Settings.h

        src/core/interface/CommandsInterface.cpp
        src/core/interface/CommandsInterface.h
        src/core/interface/BaseFIFOInterface.h
        src/core/commands/Command.cpp
        src/core/commands/Command.h

        src/tests/interface/CommandsParserTests.cpp src/tests/io/routing_tables/SetOperationTests.cpp src/tests/db/fields/TrustLineDirectionColumnTests.cpp)

add_executable(geo_network_client ${SOURCE_FILES})

find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(geo_network_client
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}

        common
        exceptions

        db
        db_tests

        io_routing_tables
        io_routing_tables_tests

        network
)
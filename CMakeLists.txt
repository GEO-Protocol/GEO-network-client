cmake_minimum_required(VERSION 3.6)
project(geo_network_client)

# todo: change in the release version
add_definitions(-DDEBUG -DINTERNAL_ARGUMENTS_VALIDATION)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Boost COMPONENTS system filesystem thread REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

#
# source files
#
add_subdirectory(src/core/common/)
add_subdirectory(src/core/common/exceptions)
add_subdirectory(src/core/db)
#add_subdirectory(src/tests/db)
add_subdirectory(src/core/io/routing_tables)
add_subdirectory(src/tests/io/routing_tables)

add_subdirectory(src/core/io/trust_lines)
#add_subdirectory(src/tests/io/trust_lines)
add_subdirectory(src/core/logger)
add_subdirectory(src/core/interface)
add_subdirectory(src/core/network)
add_subdirectory(src/core/settings)
add_subdirectory(src/core/transactions)
add_subdirectory(src/core/trust_lines)


set(SOURCE_FILES
        main.cpp

        src/core/Core.cpp
        src/core/Core.h

        # libs
        src/libs/json/json.h)

add_executable(geo_network_client ${SOURCE_FILES})

target_link_libraries(geo_network_client
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        Boost::thread

        common
        exceptions
        logger
        settings
        network
        interface__commands
        interface__results
        io__trust_lines_storage
        io__routing_tables_storage
#        io__routing_tables_tests

        trust_lines
        transactions
)
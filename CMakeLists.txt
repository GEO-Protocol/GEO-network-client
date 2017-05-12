cmake_minimum_required(VERSION 3.6)
project(geo_network_client)

find_package(Boost COMPONENTS system filesystem thread date_time REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")



#
# Platform build options
# (supported platforms are: LINUX | MAC_OS)
#
set(PLATFORM "LINUX")

IF (${PLATFORM} MATCHES "MAC_OS")
    add_definitions(-DMAC_OS)
ENDIF (${PLATFORM} MATCHES "MAC_OS")

IF (${PLATFORM} MATCHES "LINUX")
    add_definitions(-DLINUX)
ENDIF (${PLATFORM} MATCHES "LINUX")

#
# Debug options
# (available options are: DEBUG | RELEASE)
#
set(BUILD_CONFIGURATION "DEBUG")

IF (${BUILD_CONFIGURATION} MATCHES "DEBUG")
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")

    add_definitions(
        -DDEBUG
        -DINTERNAL_ARGUMENTS_VALIDATION
    )

    #
    # Debug logs configuration
    #
    add_definitions(
        -DDEBUG_LOG_NETWORK_COMMUNICATOR
        # -DDEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR

        -DDEBUG_LOG_SCHEDULER # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_MAX_FLOW_CALCULATION # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_STORAGE_HANDLER # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_PATHS_COLLECTING # ToDo: change me in relevant transaction code
    )

ENDIF (${BUILD_CONFIGURATION} MATCHES "DEBUG")

IF (${BUILD_CONFIGURATION} MATCHES "RELEASE")
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Os -s -fdata-sections -fmerge-all-constants")
ENDIF (${BUILD_CONFIGURATION} MATCHES "RELEASE")

#
# Engine type options
# (available options are:
#   DC - indicates that this build sould be used in datacenter,
#   DECENTRALIZED - indicates that this build would be used as a standalone node in a public network)
#
set(ENGINE_TYPE "DC")

IF (${ENGINE_TYPE} MATCHES "DC")
    add_definitions(-DENGINE_TYPE_DC)
ENDIF (${ENGINE_TYPE} MATCHES "DC")

IF (${ENGINE_TYPE} MATCHES "DECENTRALIZED")
    add_definitions(-DENGINE_TYPE_DECENTRALIZED)
ENDIF (${ENGINE_TYPE} MATCHES "DECENTRALIZED")


#
# Logs configuration
#
add_definitions(-DTRANSACTIONS_LOG)


#
# Boost configuration
#
set(Boost_USE_SHARED_LIBS ON)
set(Boost_USE_MULTITHREADED ON)


#
# Source files
#
add_subdirectory(src/core/common/)
add_subdirectory(src/core/common/exceptions)
add_subdirectory(src/core/logger)
add_subdirectory(src/core/interface)
add_subdirectory(src/core/network)
add_subdirectory(src/core/settings)
add_subdirectory(src/core/paths)
add_subdirectory(src/core/transactions)
add_subdirectory(src/core/trust_lines)
add_subdirectory(src/core/max_flow_calculation)
add_subdirectory(src/core/delayed_tasks)
add_subdirectory(src/core/payments/amount_blocks)
add_subdirectory(src/core/io/storage)
add_subdirectory(src/core/resources)


#
# Libs
#
add_subdirectory(src/libs/sqlite3)

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
        libsqlite3

        logger
        settings

        network__communicator
        network__messages

        interface__commands
        interface__results

        paths
        trust_lines
        max_flow_calculation
        delayed_tasks
        transactions
        resources_manager)

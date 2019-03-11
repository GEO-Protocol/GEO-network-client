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
# (available options are: DEBUG | RELEASE | INTERNAL_TESTS | EXTERNAL_TESTS)
#
set(BUILD_CONFIGURATION "RELEASE")

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
        # Subsystems
        -DDEBUG_LOG_NETWORK_COMMUNICATOR
#        -DDEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        -DDEBUG_LOG_STORAGE_HANDLER # ToDo: change me in relevant transaction code

        # Transactions control
        -DDEBUG_LOG_TRANSACTIONS_MANAGER
        -DDEBUG_LOG_SCHEDULER # ToDo: change me in relevant transaction code

        # Transactions
        -DDEBUG_LOG_TRUST_LINES_PROCESSING
        -DDEBUG_LOG_ROUTING_TABLES_PROCESSING
        -DDEBUG_LOG_MAX_FLOW_CALCULATION
        -DDEBUG_LOG_PATHS_COLLECTING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_PAYMENTS_PROCESSING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_CYCLES_PROCESSING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_CYCLES_BUILDING_POCESSING
        -DDEBUG_LOG_OBSEVING_HANDLER
    )

ENDIF (${BUILD_CONFIGURATION} MATCHES "DEBUG")

IF (${BUILD_CONFIGURATION} MATCHES "RELEASE")
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Os -s -fdata-sections -fmerge-all-constants")
ENDIF (${BUILD_CONFIGURATION} MATCHES "RELEASE")

IF (${BUILD_CONFIGURATION} MATCHES "INTERNAL_TESTS")
    add_subdirectory(src/tests)

    add_definitions(
        -DINTERNAL_TESTS
        -DINTERNAL_ARGUMENTS_VALIDATION
    )

    #
    # Logs configuration
    #
    add_definitions(
        # Subsystems
        -DDEBUG_LOG_NETWORK_COMMUNICATOR
        # -DDEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        -DDEBUG_LOG_STORAGE_HANDLER # ToDo: change me in relevant transaction code

        # Transactions control
        -DDEBUG_LOG_TRANSACTIONS_MANAGER
        -DDEBUG_LOG_SCHEDULER # ToDo: change me in relevant transaction code
        # Transactions
        -DDEBUG_LOG_TRUST_LINES_PROCESSING
        -DDEBUG_LOG_ROUTING_TABLES_PROCESSING
        -DDEBUG_LOG_MAX_FLOW_CALCULATION
        -DDEBUG_LOG_PATHS_COLLECTING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_PAYMENTS_PROCESSING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_CYCLES_PROCESSING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_CYCLES_BUILDING_POCESSING
        -DDEBUG_LOG_OBSEVING_HANDLER
    )

    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
ENDIF (${BUILD_CONFIGURATION} MATCHES "INTERNAL_TESTS")

IF (${BUILD_CONFIGURATION} MATCHES "EXTERNAL_TESTS")

    add_definitions(
        -DTESTS
        -DINTERNAL_ARGUMENTS_VALIDATION
    )

    #
    # Logs configuration
    #
    add_definitions(
        # Subsystems
        -DDEBUG_LOG_NETWORK_COMMUNICATOR
#       -DDEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        -DDEBUG_LOG_STORAGE_HANDLER # ToDo: change me in relevant transaction code

        # Transactions control
        -DDEBUG_LOG_TRANSACTIONS_MANAGER
        -DDEBUG_LOG_SCHEDULER # ToDo: change me in relevant transaction code

        # Transactions
        -DDEBUG_LOG_TRUST_LINES_PROCESSING
        -DDEBUG_LOG_ROUTING_TABLES_PROCESSING
        -DDEBUG_LOG_MAX_FLOW_CALCULATION
        -DDEBUG_LOG_PATHS_COLLECTING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_PAYMENTS_PROCESSING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_CYCLES_PROCESSING # ToDo: change me in relevant transaction code
        -DDEBUG_LOG_CYCLES_BUILDING_POCESSING
        -DDEBUG_LOG_OBSEVING_HANDLER
    )

    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Os -s -fdata-sections -fmerge-all-constants")
ENDIF (${BUILD_CONFIGURATION} MATCHES "EXTERNAL_TESTS")

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
add_subdirectory(src/core/topology)
add_subdirectory(src/core/delayed_tasks)
add_subdirectory(src/core/payments/reservations)
add_subdirectory(src/core/io/storage)
add_subdirectory(src/core/resources)
add_subdirectory(src/core/cycles)
add_subdirectory(src/core/subsystems_controller)
add_subdirectory(src/core/equivalents)
add_subdirectory(src/core/crypto)
add_subdirectory(src/core/contractors)
add_subdirectory(src/core/observing)


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
        -lsodium

        logger
        settings

        network__communicator
        network__messages

        interface__commands
        interface__results
        interface__events

        paths
        trust_lines
        topology
        delayed_tasks
        transactions
        resources_manager
        cycles
        subsystems_controller
        equivalents
        crypto

        contractors
        observing)

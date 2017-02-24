cmake_minimum_required(VERSION 3.6)
project(geo_network_client)


# Logs configuration
add_definitions(-DTRANSACTIONS_LOG)

# Debug logs configuration
add_definitions(-DNETWORK_DEBUG_LOG)

# Scheduler debug log
add_definitions(-DSCHEDULER_DEBUG_LOG)

# todo: change in the release version

# Available options are: DEBUG | RELEASE | REALEASE_WITH_DEBUG_LOG
set(BUILD_CONFIGURATION "DEBUG")


IF (${BUILD_CONFIGURATION} MATCHES "DEBUG")
    add_definitions(-DDEBUG -DINTERNAL_ARGUMENTS_VALIDATION)
ENDIF (${BUILD_CONFIGURATION} MATCHES "DEBUG")

IF (${BUILD_CONFIGURATION} MATCHES "REALEASE_WITH_DEBUG_LOG")
    add_definitions(-DDEBUG_LOG)
ENDIF (${BUILD_CONFIGURATION} MATCHES "REALEASE_WITH_DEBUG_LOG")

IF (${BUILD_CONFIGURATION} MATCHES "DEBUG")
    add_definitions(-DSCHEDULER_DEBUG_LOG)
ENDIF (${BUILD_CONFIGURATION} MATCHES "DEBUG")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")


IF (${BUILD_CONFIGURATION} MATCHES "DEBUG")
    set(CMAKE_BUILD_TYPE "DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -lboost_date_time")
ENDIF (${BUILD_CONFIGURATION} MATCHES "DEBUG")

IF (${BUILD_CONFIGURATION} MATCHES "RELEASE")
    set(CMAKE_BUILD_TYPE "RELEASE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Os -s -fdata-sections -fmerge-all-constants")
ENDIF (${BUILD_CONFIGURATION} MATCHES "RELEASE")


set(Boost_USE_SHARED_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system filesystem thread date_time REQUIRED)


#
# source files
#
add_subdirectory(src/core/common/)
add_subdirectory(src/core/common/exceptions)
add_subdirectory(src/core/common/time)
add_subdirectory(src/core/common/memory)
add_subdirectory(src/core/common/multiprecision)
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
add_subdirectory(src/core/paths)
add_subdirectory(src/core/transactions)
add_subdirectory(src/core/trust_lines)
add_subdirectory(src/core/max_flow_calculation)

add_subdirectory(src/core/payments/amount_blocks)

set(SOURCE_FILES
        main.cpp

        src/core/Core.cpp
        src/core/Core.h

        # libs
        src/libs/json/json.h src/core/delayed_tasks/Cycles.h src/core/delayed_tasks/Cycles.cpp)

add_executable(geo_network_client ${SOURCE_FILES})

target_link_libraries(geo_network_client
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        Boost::thread

#        common
#        exceptions
#        time
#        memory
#        multiprecision
        logger
        settings
        network
        interface__commands
        interface__results

        io__trust_lines_storage
        io__routing_tables_storage

        paths
        trust_lines
        max_flow_calculation
        transactions
)

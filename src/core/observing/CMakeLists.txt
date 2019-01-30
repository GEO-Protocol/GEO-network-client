cmake_minimum_required(VERSION 3.6)
find_package(Boost COMPONENTS system REQUIRED)


add_subdirectory(messages)

set(SOURCE_FILES
        ObservingHandler.h
        ObservingHandler.cpp

        ObservingTransaction.h
        ObservingTransaction.cpp

        ObservingCommunicator.h
        ObservingCommunicator.cpp)


add_library(observing ${SOURCE_FILES})
target_link_libraries(observing
        observing__messages
        ${Boost_SYSTEM_LIBRARY})
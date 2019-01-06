cmake_minimum_required(VERSION 3.6)
find_package(Boost COMPONENTS system REQUIRED)


set(SOURCE_FILES
        ObservingCommunicator.h
        ObservingCommunicator.cpp)


add_library(observing__communicator ${SOURCE_FILES})
target_link_libraries(observing__communicator
        ${Boost_SYSTEM_LIBRARY})

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Settings.h
        Settings.cpp)

add_library(settings ${SOURCE_FILES})
target_link_libraries(settings
        common)
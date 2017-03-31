cmake_minimum_required(VERSION 3.6)

add_subdirectory(lib)

set(SOURCE_FILES
        PathsManager.h
        PathsManager.cpp)

add_library(paths ${SOURCE_FILES})

target_link_libraries(paths
        slib_paths)

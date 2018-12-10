cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Path.h
        Path.cpp
        PathNew.h
        PathNew.cpp
        PathsCollection.h
        PathsCollection.cpp)

add_library(slib_paths ${SOURCE_FILES})

target_link_libraries(slib_paths)

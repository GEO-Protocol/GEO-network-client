cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
    NodeUUID.cpp
    NodeUUID.h
    Types.h)

add_library(common ${SOURCE_FILES})
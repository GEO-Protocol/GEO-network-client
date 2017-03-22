cmake_minimum_required(VERSION 3.6)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/libs/sqlite3)
set(SOURCE_FILES
        sqlite3.c
        sqlite3.h
        sqlite3ext.h)

add_library(libsqlite3 ${SOURCE_FILES})
target_link_libraries(libsqlite3
        ${CMAKE_DL_LIBS})

# This file is part of GEO Protocol.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
#
# No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/libs/sqlite3)
set(SOURCE_FILES
        sqlite3.c
        sqlite3.h
        sqlite3ext.h)

add_library(libsqlite3 ${SOURCE_FILES})
target_link_libraries(libsqlite3
        ${CMAKE_DL_LIBS})

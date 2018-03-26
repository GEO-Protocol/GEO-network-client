# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

add_subdirectory(lib)

set(SOURCE_FILES
        PathsManager.h
        PathsManager.cpp)

add_library(paths ${SOURCE_FILES})

target_link_libraries(paths
        slib_paths)

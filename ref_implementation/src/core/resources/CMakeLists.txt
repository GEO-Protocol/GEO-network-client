# This file is part of GEO Protocol.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
#
# No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        manager/ResourcesManager.h
        manager/ResourcesManager.cpp
        resources/BaseResource.h
        resources/BaseResource.cpp
        resources/PathsResource.h
        resources/PathsResource.cpp)

add_library(resources_manager ${SOURCE_FILES})

target_link_libraries(resources_manager)

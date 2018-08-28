# This file is part of GEO Protocol.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
#
# No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core/interface/)
set(SOURCE_FILES
        interface/ResultsInterface.cpp
        interface/ResultsInterface.h

        result/CommandResult.cpp
        result/CommandResult.h)

add_library(interface__results ${SOURCE_FILES})
target_link_libraries(interface__results
        common
        #        time
        exceptions)

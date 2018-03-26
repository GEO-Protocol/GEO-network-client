# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h
        ThreeNodes/CyclesThreeNodesBalancesRequestMessage.cpp
        ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h
        ThreeNodes/CyclesThreeNodesBalancesResponseMessage.cpp
        FourNodes/CyclesFourNodesBalancesRequestMessage.h
        FourNodes/CyclesFourNodesBalancesRequestMessage.cpp
        FourNodes/CyclesFourNodesBalancesResponseMessage.h
        FourNodes/CyclesFourNodesBalancesResponseMessage.cpp
        SixAndFiveNodes/base/CyclesBaseFiveOrSixNodesInBetweenMessage.h
        SixAndFiveNodes/base/CyclesBaseFiveOrSixNodesInBetweenMessage.cpp
        SixAndFiveNodes/base/CyclesBaseFiveOrSixNodesBoundaryMessage.h
        SixAndFiveNodes/base/CyclesBaseFiveOrSixNodesBoundaryMessage.cpp
        SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp
        SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp
        SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp
        SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp
        )

add_library(messages__cycles ${SOURCE_FILES})
target_link_libraries(messages__cycles

        messages__base__transaction

        common
        exceptions)

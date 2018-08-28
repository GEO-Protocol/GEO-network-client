# This file is part of GEO Protocol.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
#
# No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        AddNodeToBlackListTransaction.h
        AddNodeToBlackListTransaction.cpp

        GetBlackListTransaction.h
        GetBlackListTransaction.cpp

        RemoveNodeFromBlackListTransaction.h
        RemoveNodeFromBlackListTransaction.cpp

        CheckIfNodeInBlackListTransaction.h
        CheckIfNodeInBlackListTransaction.cpp)

add_library(transactions__black_list ${SOURCE_FILES})

target_link_libraries(transactions__black_list
        transactions__base
        io__storage)

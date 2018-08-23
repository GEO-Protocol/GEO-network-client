# This file is part of GEO Protocol.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
    SetOutgoingTrustLineTransaction.h
    SetOutgoingTrustLineTransaction.cpp

    SetIncomingTrustLineTransaction.h
    SetIncomingTrustLineTransaction.cpp

    CloseIncomingTrustLineTransaction.h
    CloseIncomingTrustLineTransaction.cpp

    CloseOutgoingTrustLineTransaction.h
    CloseOutgoingTrustLineTransaction.cpp

    RejectOutgoingTrustLineTransaction.h
    RejectOutgoingTrustLineTransaction.cpp)

add_library(transactions__trustlines ${SOURCE_FILES})
target_link_libraries(transactions__trustlines
    transactions__base
    common
    exceptions
    network__messages)

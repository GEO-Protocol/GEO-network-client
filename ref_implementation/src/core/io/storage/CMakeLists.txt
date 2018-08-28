# This file is part of GEO Protocol.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
#
# No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        StorageHandler.h
        StorageHandler.cpp
        TrustLineHandler.h
        TrustLineHandler.cpp
        PaymentOperationStateHandler.h
        PaymentOperationStateHandler.cpp
        TransactionsHandler.h
        TransactionsHandler.cpp
        HistoryStorage.h
        HistoryStorage.cpp
        record/base/Record.h
        record/base/Record.cpp
        record/trust_line/TrustLineRecord.h
        record/trust_line/TrustLineRecord.cpp
        record/payment/PaymentRecord.h
        record/payment/PaymentRecord.cpp

        IOTransaction.cpp
        IOTransaction.h

        CommunicatorMessagesQueueHandler.h
        CommunicatorMessagesQueueHandler.cpp

        CommunicatorStorageHandler.h
        CommunicatorStorageHandler.cpp

        CommunicatorIOTransaction.h
        CommunicatorIOTransaction.cpp

        BlackListHandler.h
        BlackListHandler.cpp

        NodeFeaturesHandler.h
        NodeFeaturesHandler.cpp)

add_library(io__storage ${SOURCE_FILES})

target_link_libraries(io__storage
        libsqlite3)

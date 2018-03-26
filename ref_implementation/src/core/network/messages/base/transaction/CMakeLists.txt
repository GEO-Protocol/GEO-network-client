# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TransactionMessage.cpp
        TransactionMessage.h

        ConfirmationMessage.h
        ConfirmationMessage.cpp

        DestinationMessage.h
        DestinationMessage.cpp)

add_library(messages__base__transaction ${SOURCE_FILES})
target_link_libraries(messages__base__transaction

        common
        exceptions)

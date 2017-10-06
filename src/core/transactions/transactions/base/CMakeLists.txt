cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TransactionUUID.h

        BaseTransaction.h
        BaseTransaction.cpp

        BaseCollectTopologyTransaction.h
        BaseCollectTopologyTransaction.cpp)

add_library(transactions__base ${SOURCE_FILES})
target_link_libraries(transactions__base
        common
        #        time
        exceptions

        network__messages)

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
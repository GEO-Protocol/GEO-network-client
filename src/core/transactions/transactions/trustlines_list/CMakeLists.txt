cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        GetFirstLevelContractorsTransaction.h
        GetFirstLevelContractorsTransaction.cpp

        GetTrustLinesListTransaction.h
        GetTrustLinesListTransaction.cpp

        GetTrustLineByIDTransaction.h
        GetTrustLineByIDTransaction.cpp
        GetTrustLineByAddressTransaction.h
        GetTrustLineByAddressTransaction.cpp

        GetEquivalentListTransaction.h
        GetEquivalentListTransaction.cpp)

add_library(transactions__trustlines_list ${SOURCE_FILES})
target_link_libraries(transactions__trustlines_list
        transactions__base)
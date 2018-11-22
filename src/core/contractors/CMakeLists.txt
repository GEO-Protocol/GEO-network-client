cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Contractor.h
        Contractor.cpp
        ContractorsManager.h
        ContractorsManager.cpp

        addresses/BaseAddress.h
        addresses/BaseAddress.cpp
        addresses/IPv4WithPortAddress.h
        addresses/IPv4WithPortAddress.cpp)

add_library(contractors ${SOURCE_FILES})
target_link_libraries(contractors
        common)
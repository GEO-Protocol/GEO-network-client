cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Contractor.h
        Contractor.cpp
        ContractorsManager.h
        ContractorsManager.cpp

        addresses/IPv4Address.h
        addresses/IPv4Address.cpp)

add_library(contractors ${SOURCE_FILES})
target_link_libraries(contractors
        common)
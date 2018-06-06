cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES

        CryptoKey.h
        CryptoKey.cpp KeyChain.cpp KeyChain.h)

add_library(crypto ${SOURCE_FILES})
target_link_libraries(crypto)
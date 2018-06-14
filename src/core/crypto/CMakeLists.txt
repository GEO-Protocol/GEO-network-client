cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -lsodium")

set(SOURCE_FILES

        memory.h
        memory.cpp

        keychain.h
        keychain.cpp

        lamportkeys.cpp
        lamportkeys.h

        lamportscheme.cpp
        lamportscheme.h)

add_library(crypto ${SOURCE_FILES})
target_link_libraries(
        ${sodium}
        crypto)




cmake_minimum_required(VERSION 3.6)
find_package(Boost COMPONENTS system REQUIRED)


set(SOURCE_FILES
        ProvidingHandler.h
        ProvidingHandler.cpp

        Provider.h
        Provider.cpp

        )

add_library(providing ${SOURCE_FILES})
target_link_libraries(providing
        ${Boost_SYSTEM_LIBRARY})
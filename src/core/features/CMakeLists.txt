cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        FeaturesManager.h
        FeaturesManager.cpp)

add_library(features ${SOURCE_FILES})
target_link_libraries(features)
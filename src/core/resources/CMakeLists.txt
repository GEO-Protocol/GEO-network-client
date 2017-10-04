cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        manager/ResourcesManager.h
        manager/ResourcesManager.cpp
        resources/BaseResource.h
        resources/BaseResource.cpp
        resources/PathsResource.h
        resources/PathsResource.cpp)

add_library(resources_manager ${SOURCE_FILES})

target_link_libraries(resources_manager)
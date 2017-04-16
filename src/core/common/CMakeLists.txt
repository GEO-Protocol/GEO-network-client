cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
    NodeUUID.cpp
    NodeUUID.h
    Types.h serialization/BytesDeserializer.cpp serialization/BytesDeserializer.h serialization/BytesSerializer.cpp serialization/BytesSerializer.h Constraints.h)

add_library(common ${SOURCE_FILES})
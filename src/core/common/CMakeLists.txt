cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
    Types.h 
    Constraints.h
    
    NodeUUID.cpp
    NodeUUID.h
    
    serialization/BytesDeserializer.cpp 
    serialization/BytesDeserializer.h 
    serialization/BytesSerializer.cpp 
    serialization/BytesSerializer.h 
    
    time/TimeUtils.h
    multiprecision/MultiprecisionUtils.h
    memory/MemoryUtils.h)

add_library(common ${SOURCE_FILES})

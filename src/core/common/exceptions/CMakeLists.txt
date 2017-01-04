cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        CommandParsingError.h
        ConflictError.h
        Exception.cpp
        Exception.h
        IndexError.h
        IOError.h
        MemoryError.h
        NotFoundError.h
        OverflowError.h
        PreconditionFailedError.h
        RuntimeError.h
        ValueError.h)
add_library(exceptions SHARED ${SOURCE_FILES})
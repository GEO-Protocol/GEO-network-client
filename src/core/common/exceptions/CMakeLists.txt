cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ConflictError.h
        Exception.cpp
        Exception.h
        IndexError.h
        IOError.h
        MemoryError.h
        NotFoundError.h
        OverflowError.h
        ValueError.h RuntimeError.h)
add_library(exceptions ${SOURCE_FILES})
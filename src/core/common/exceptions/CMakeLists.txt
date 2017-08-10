cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        Exception.cpp
        Exception.h
        CommandParsingError.h
        ConflictError.h
        IndexError.h
        IOError.h
        MemoryError.h
        NotFoundError.h
        OverflowError.h
        PreconditionFailedError.h
        RuntimeError.h
        ValueError.h
        CallChainBreakException.h)

add_library(exceptions ${SOURCE_FILES})
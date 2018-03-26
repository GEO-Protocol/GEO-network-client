# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

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

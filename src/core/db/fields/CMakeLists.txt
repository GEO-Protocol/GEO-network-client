cmake_minimum_required(VERSION 3.6)


set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system filesystem REQUIRED)


set(SOURCE_FILES
        common/AbstractFileDescriptorHandler.cpp
        common/AbstractFileDescriptorHandler.h
        common/AbstractRecordsHandler.cpp
        common/AbstractRecordsHandler.h
        common/RecordNumbersIndex.cpp
        common/RecordNumbersIndex.h

        tl_direction_column/TrustLineDirectionColumn.cpp
        tl_direction_column/TrustLineDirectionColumn.h

        uuid_column/internal/BucketBlock.cpp
        uuid_column/internal/BucketBlock.h
        uuid_column/internal/BucketBlockRecord.cpp
        uuid_column/internal/BucketBlockRecord.h
        uuid_column/UUIDColumn.cpp
        uuid_column/UUIDColumn.h)

add_library(db__fields ${SOURCE_FILES})
target_link_libraries(db__fields
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}

        exceptions
        common)
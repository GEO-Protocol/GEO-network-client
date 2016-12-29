cmake_minimum_required(VERSION 3.6)

find_package(Boost COMPONENTS system filesystem REQUIRED)

set(SOURCE_FILES
        fields/common/AbstractFileDescriptorHandler.cpp
        fields/common/AbstractFileDescriptorHandler.h
        fields/common/AbstractRecordsHandler.cpp
        fields/common/AbstractRecordsHandler.h
        fields/common/RecordNumbersIndex.cpp
        fields/common/RecordNumbersIndex.h
        fields/tl_direction_column/TrustLineDirectionColumn.cpp
        fields/tl_direction_column/TrustLineDirectionColumn.h
        fields/uuid_column/internal/BucketBlock.cpp
        fields/uuid_column/internal/BucketBlock.h
        fields/uuid_column/internal/BucketBlockRecord.cpp
        fields/uuid_column/internal/BucketBlockRecord.h
        fields/uuid_column/UUIDColumn.cpp
        fields/uuid_column/UUIDColumn.h)

add_library(db ${SOURCE_FILES})
target_link_libraries(db
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}

        common)
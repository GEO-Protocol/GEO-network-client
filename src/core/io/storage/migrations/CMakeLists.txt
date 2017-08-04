cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        AbstractMigration.h
        AbstractMigration.cpp

        AmountAndBalanceSerialization.h
        AmountAndBalanceSerialization.cpp

        SolomonHistoryMigration.h
        SolomonHistoryMigration.cpp

        SolomonHistoryMigrationTwo.h
        SolomonHistoryMigrationTwo.cpp
        SolomonHistoryMigration.cpp

        UniqueIndexHistoryMigration.h
        UniqueIndexHistoryMigration.cpp

        MaxDemianMigration.h
        MaxDemianMigration.cpp
        
        MaxDemianSecondMigration.h 
        MaxDemianSecondMigration.cpp

        MaxDemianThirdMigration.h
        MaxDemianThirdMigration.cpp DeleteSerializedTransactionsMigration.h DeleteSerializedTransactionsMigration.cpp PositiveSignMigration.h PositiveSignMigration.cpp)

add_library(migrations ${SOURCE_FILES})

target_link_libraries(migrations
        libsqlite3
        common
        exceptions)
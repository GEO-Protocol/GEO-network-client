#include "src/core/Core.h"

//#define INTERNAL_ARGUMENTS_VALIDATION


#ifdef TESTS
//#define ROUTING_TABLE_TESTS
//#define DB_TRUST_LINE_DIRECTION_COLUMN_TESTS
#define DB_UUID_COLUMN_TESTS
#endif



//#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
//#include "src/tests/interface/OperationsLogTests.cpp"
//#include "src/tests/db/routing_tables/BucketBlockRecordTests.cpp"
//#include "src/tests/db/routing_tables/BucketBlockTests.cpp"
//#include "src/tests/db/uuid_column/UUIDMapColumnTests.cpp"


#ifdef ROUTING_TABLE_TESTS
#include "src/tests/io/routing_tables/OperationsLogTests.cpp"
#include "src/tests/io/routing_tables/SetOperationTests.cpp"
#include "src/tests/io/routing_tables/RemoveOperationTests.cpp"
#include "src/tests/io/routing_tables/DirectionUpdateOperationTests.cpp"
#endif

#ifdef DB_TRUST_LINE_DIRECTION_COLUMN_TESTS
#include "src/tests/db/fields/TrustLineDirectionColumnTests.cpp"
#endif

#ifdef DB_UUID_COLUMN_TESTS
#include "src/tests/db/fields/UUIDMapColumnTests.cpp"
#endif




int main() {
//    FileBackedMessagesQueueTests fileBackedMessagesQueueTests;
//    fileBackedMessagesQueueTests.run();

//    OperationsLogTests commandsParserTests;
//    commandsParserTests.run();

//    db::routing_tables::BucketBlockRecordTests bucketBlockRecordTests;
//    bucketBlockRecordTests.run();

//    db::routing_tables::BucketBlockTests bucketBlockTests;
//    bucketBlockTests.run();

//    db::fields::UUIDMapColumnTests mUUIDMapColumnTests;
//    mUUIDMapColumnTests.run();


#ifdef ROUTING_TABLE_TESTS
    {
        OperationsLogTests operationsLogTests;
        operationsLogTests.run();

        SetOperationsTests setOperationsTests;
        setOperationsTests.run();

        RemoveOperationsTests removeOperationsTests;
        removeOperationsTests.run();

        DirectionUpdateOperationsTests directionUpdateOperationsTests;
        directionUpdateOperationsTests.run();
    }
#endif


#ifdef DB_TRUST_LINE_DIRECTION_COLUMN_TESTS
    {
        TrustLineDirectionColumnTests trustLineDirectionColumnTests;
        trustLineDirectionColumnTests.run();
    }
#endif

#ifdef DB_UUID_COLUMN_TESTS
    {
        UUIDMapColumnTests uuidMapColumnTests;
        uuidMapColumnTests.run();
    }
#endif


#ifndef TESTS
    return Core().run();
#endif
}
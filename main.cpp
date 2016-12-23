#include "src/core/Core.h"

//#define INTERNAL_ARGUMENTS_VALIDATION
#ifdef TESTS
//#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
//#include "src/tests/interface/OperationsLogTests.cpp"
//#include "src/tests/db/routing_tables/BucketBlockRecordTests.cpp"
//#include "src/tests/db/routing_tables/BucketBlockTests.cpp"
//#include "src/tests/db/uuid_map_column/UUIDMapColumnTests.cpp"

// io/routing_tables
#include "src/tests/io/routing_tables/OperationsLogTests.cpp"
#include "src/tests/io/routing_tables/SetOperationTests.cpp"
#include "src/tests/io/routing_tables/RemoveOperationTests.cpp"
#include "src/tests/io/routing_tables/DirectionUpdateOperationTests.cpp"

#endif


int main() {
#ifdef TESTS
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


    {
        // io/routing_tables/

        OperationsLogTests operationsLogTests;
        operationsLogTests.run();

//        SetOperationsTests setOperationsTests;
//        setOperationsTests.run();
//
//        RemoveOperationsTests removeOperationsTests;
//        removeOperationsTests.run();

        DirectionUpdateOperationsTests directionUpdateOperationsTests;
        directionUpdateOperationsTests.run();
    }


#endif

#ifndef TESTS
    return Core().run();
#endif
}
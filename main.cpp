#include "src/core/Core.h"


#define TESTS


#ifdef TESTS
//#define TESTS__DB__UUID_COLUMN
//#define TESTS__DB__TRUST_LINE_DIRECTION_COLUMN
#define TESTS__ROUTING_TABLE

#endif


// todo: move this into separate if..def
//#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
//#include "src/tests/parseInterface/OperationsLogTests.cpp"
//#include "src/tests/db/routing_tables/BucketBlockRecordTests.cpp"
//#include "src/tests/db/routing_tables/BucketBlockTests.cpp"
//#include "src/tests/db/uuid_column/UUIDMapColumnTests.cpp"


#ifdef TESTS__DB__UUID_COLUMN
#include "src/tests/db/fields/uuid_column/UUIDColumnTests.cpp"
#endif

#ifdef TESTS__DB__TRUST_LINE_DIRECTION_COLUMN
#include "src/tests/db/fields/tl_direction_column/TrustLineDirectionColumnTests.cpp"
#endif


#ifdef TESTS__ROUTING_TABLE
#include "src/tests/io/routing_tables/OperationsLogTests.cpp"
#include "src/tests/io/routing_tables/operations/SetOperationTests.cpp"
#include "src/tests/io/routing_tables/operations/RemoveOperationTests.cpp"
#include "src/tests/io/routing_tables/operations/DirectionUpdateOperationTests.cpp"
#include "src/tests/io/routing_tables/RoutingTableTests.cpp"
#endif


int main() {
    // todo: include other tests here

#ifdef TESTS__DB__UUID_COLUMN
    UUIDColumnTests tests;
    tests.run();
#endif

#ifdef TESTS__DB__TRUST_LINE_DIRECTION_COLUMN
    TrustLineDirectionColumnTests tests;
    tests.run();
#endif

#ifdef TESTS__ROUTING_TABLE
    {
        OperationsLogTests tests;
        tests.run();
    }
    {
        SetOperationsTests tests;
        tests.run();
    }
    {
        RemoveOperationsTests tests;
        tests.run();
    }
    {
        DirectionUpdateOperationsTests tests;
        tests.run();
    }
    {
        AbstractRoutingTableTests tests;
        tests.run();
    }
#endif


#ifndef TESTS
    return Core().run();
#endif
}
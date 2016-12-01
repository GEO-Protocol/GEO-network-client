#include "src/core/Core.h"

#ifdef TESTS
//#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
//#include "src/tests/interface/CommandsParserTests.cpp"
#include "src/tests/db/routing_tables/BucketBlockRecordTests.cpp"
#endif

int main() {
#ifdef TESTS
//    FileBackedMessagesQueueTests fileBackedMessagesQueueTests;
//    fileBackedMessagesQueueTests.run();

//    CommandsParserTests commandsParserTests;
//    commandsParserTests.run();

    db::routing_tables::BucketBlockRecordTests bucketBlockRecordTests;
    bucketBlockRecordTests.run();
#endif

#ifndef TESTS
    return Core().run();
#endif
}
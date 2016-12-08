#include "src/core/Core.h"

#ifdef TESTS
#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
#include "src/tests/interface/CommandsParserTests.cpp"
#include "src/tests/trust_lines/UUIDMapBlockStorageTest.h"
#include "src/core/network/UUID2IP.h"
#endif

int main() {
#ifdef TESTS
//    FileBackedMessagesQueueTests fileBackedMessagesQueueTests;
//    fileBackedMessagesQueueTests.run();
//    CommandsParserTests commandsParserTests;
//    commandsParserTests.run();
    UUIDMapBlockStorageTest mapBlockStorageTest;
    mapBlockStorageTest.start();
#endif

#ifndef TESTS
    return Core().run();
#endif
}
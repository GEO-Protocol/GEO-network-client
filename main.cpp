#include "src/core/Core.h"

#ifdef TESTS
#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
#include "src/tests/interface/CommandsParserTests.cpp"
#include "src/tests/trust_lines/UUIDMapBlockStorageTest.h"
#include "src/tests/trust_lines/TrustLinesManagerTest.h"
#include "src/core/network/UUID2IP.h"
#endif

int main() {
#ifdef TESTS
//    FileBackedMessagesQueueTests fileBackedMessagesQueueTests;
//    fileBackedMessagesQueueTests.run();
//    CommandsParserTests commandsParserTests;
//    commandsParserTests.run();
    TrustLinesManagerTest trustLinesManagerTest;
    trustLinesManagerTest.run();
#endif

#ifndef TESTS
    return Core().run();
#endif
}
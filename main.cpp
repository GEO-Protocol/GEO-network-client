#include "src/core/Core.h"

#ifdef TESTS
#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
#include "src/tests/interface/CommandsParserTests.cpp"
#endif

int main() {
#ifdef TESTS
//    FileBackedMessagesQueueTests fileBackedMessagesQueueTests;
//    fileBackedMessagesQueueTests.run();

    CommandsParserTests commandsParserTests;
    commandsParserTests.run();
#endif

#ifndef TESTS
    return Core().run();
#endif
}
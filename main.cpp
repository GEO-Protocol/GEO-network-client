#include "src/core/Core.h"

#ifdef TESTS
#include "src/tests/network/FileBackedMessagesQueueTests.cpp"
#include "src/tests/interface/CommandsParserTests.cpp"
#include "src/core/network/UUID2IP.h"
#endif

int main() {
    UUID2IP *uuid2IP = new UUID2IP("10.0.2.2", "8088");
/*#ifdef TESTS
//    FileBackedMessagesQueueTests fileBackedMessagesQueueTests;
//    fileBackedMessagesQueueTests.run();

    CommandsParserTests commandsParserTests;
    commandsParserTests.run();
#endif

#ifndef TESTS
    return Core().run();
#endif*/
}
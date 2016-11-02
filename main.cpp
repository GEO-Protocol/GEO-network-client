#include "src/core/Core.h"

#include "src/tests/network/FileBackedMessagesQueueTests.cpp"

int main() {
    bool runTests = true;

    if (runTests) {
        FileBackedMessagesQueueTests fileBackedMessagesQueueTests;
        fileBackedMessagesQueueTests.run();

    } else {
        auto core = Core();
        return core.run();
    }
}
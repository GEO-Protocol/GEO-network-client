#include "../../core/network/internal/OutgoingMessagesQueue.h"

class SimpleTestMessage: public Message {
public:
    virtual shared_ptr<SerialisedMessage> serialize() const {
        const char* message = "qqqq....qqqq....";

        const uint64_t bytesLen = 16;
        char* bytes = (char*)malloc(bytesLen);
        memcpy(bytes, message, bytesLen);

        return std::shared_ptr<SerialisedMessage>(new SerialisedMessage((uint8_t*)(bytes), bytesLen));
    }

    virtual const MessageTypeID typeID() const {
        return TestSimpleMessage;
    }
};

class LongTestMessage: public Message {
public:
    virtual shared_ptr<SerialisedMessage> serialize() const {
        const char* message = "LLLL....LLLL....LLLL....LLLL....LLLL....LLLL....LLLL....LLLL....";

        const uint64_t bytesLen = 64;
        char* bytes = (char*)malloc(bytesLen);
        memcpy(bytes, message, bytesLen);

        return std::shared_ptr<SerialisedMessage>(new SerialisedMessage((uint8_t*)(bytes), bytesLen));
    }

    virtual const MessageTypeID typeID() const {
        return TestLongMessage;
    }
};

class FileBackedMessagesQueueTests {
public:
    void run() {
        checkSequentialReading();
        checkRemoving();
        checkCacheReusing();
        checkFileIsPresentAfterQueueIsClosed();
        checkEmptyFileRemovingAfterQueueIsClosed();
    };

    void checkSequentialReading() {
        // This test performs several writes to the file cache and
        // then compares all records with the reference message.

        auto queue = FileBackedMessagesQueue("SequentialAddingQueue");
        auto testMessage = new SimpleTestMessage();
        auto serialisedMessage = testMessage->serialize();

        // Populating queue
        for (int j = 0; j < 100; ++j) {
            queue.enqueue(testMessage->serialize());
        }

        // Checking pairs
        for (int j = 0; j < 100; ++j) {
            assert(memcmp(
                serialisedMessage.get()->bytes(), queue.nextRecord().second.get()->bytes(),
                serialisedMessage.get()->bytesCount()
            ) == 0);

            queue.removeNextRecord();
        }

        // Clearing
        if (fs::exists(fs::path(queue.cacheFilePath()))) {
            fs::remove(fs::path(queue.cacheFilePath()));
        }
    }

    void checkRemoving() {
        // This test performs several writes to the file cache
        // and then removes second record from each one pair of records.
        // To check how the class is working with records of different sizes -
        // 2 kind of pairs are used in this test: short message and long message.

        auto queue = FileBackedMessagesQueue("RemovingTestQueue");
        auto shortMessage = new SimpleTestMessage();
        auto serialisedShortMessage = shortMessage->serialize();

        auto longMessage = new LongTestMessage();
        auto serialisedLongMessage = longMessage->serialize();


        // Populating queue
        for (int j = 0; j < 25; ++j) { // 100 messages in total
            queue.enqueue(shortMessage->serialize());
            queue.enqueue(shortMessage->serialize());

            queue.enqueue(longMessage->serialize());
            queue.enqueue(longMessage->serialize());
        }

        // Checking pairs
        for (int j = 0; j < 25; ++j) { // 100 messages in total
            {
                queue.removeNextRecord();
                assert(memcmp(
                    serialisedShortMessage.get()->bytes(), queue.nextRecord().second.get()->bytes(),
                    serialisedShortMessage.get()->bytesCount()
                ) == 0);
                queue.removeNextRecord();
            }

            {
                queue.removeNextRecord();
                assert(memcmp(
                    serialisedLongMessage.get()->bytes(), queue.nextRecord().second.get()->bytes(),
                    serialisedLongMessage.get()->bytesCount()
                ) == 0);
                queue.removeNextRecord();
            }
        }

        // Clearing
        if (fs::exists(fs::path(queue.cacheFilePath()))) {
            fs::remove(fs::path(queue.cacheFilePath()));
        }
    }

    void checkCacheReusing() {
        // This test performs several writes to the file cache
        // and then closes it and reopens via separate instance.
        // Then checks if all records may be read.

        {
            auto queue = FileBackedMessagesQueue("CacheReusingTestQueue");
            auto testMessage = SimpleTestMessage();
            auto serialisedMessage = testMessage.serialize();

            // Populating queue
            for (int j = 0; j < 100; ++j) {
                queue.enqueue(testMessage.serialize());
            }
        }

        {
            // Queue handler is closed.
            auto queue = FileBackedMessagesQueue("CacheReusingTestQueue");
            auto testMessage = SimpleTestMessage();
            auto serialisedMessage = testMessage.serialize();

            // Checking pairs
            for (int j = 0; j < 100; ++j) {
                assert(memcmp(
                    serialisedMessage.get()->bytes(), queue.nextRecord().second.get()->bytes(),
                    serialisedMessage.get()->bytesCount()
                ) == 0);

                queue.removeNextRecord();
            }

            // Clearing
            if (fs::exists(fs::path(queue.cacheFilePath()))) {
                fs::remove(fs::path(queue.cacheFilePath()));
            }
        }
    }

    void checkFileIsPresentAfterQueueIsClosed(){
        // If file cache contains valid records - it should not be deleted
        // after the queue handler is deleted from memory.

        auto queuePath = string();

        {
            auto queue = FileBackedMessagesQueue("FileIsPresentAfterQueueIsClosedTestQueue");
            queuePath = queue.cacheFilePath();

            // Populating queue
            auto testMessage = new SimpleTestMessage();
            for (int j = 0; j < 10; ++j) {
                queue.enqueue(testMessage->serialize());
            }
        }

        // Queue is removed from the memory.
        // Check if file with data is present.
        assert(fs::exists(fs::path(queuePath)));

        // Clearing
        if (fs::exists(fs::path(queuePath))) {
            fs::remove(fs::path(queuePath));
        }
    }

    void checkEmptyFileRemovingAfterQueueIsClosed(){
        // If file cache contains valid records - it should not be deleted
        // after the queue handler is deleted from memory.

        auto queuePath = string();

        {
            auto queue = FileBackedMessagesQueue("EmptyFileIsRemovingAfterQueueIsClosedTestQueue");
            queuePath = queue.cacheFilePath();

            // Populating queue
            auto testMessage = new SimpleTestMessage();
            for (int j = 0; j < 10; ++j) {
                queue.enqueue(testMessage->serialize());
            }

            // Removing records
            for (int j = 0; j < 10; ++j) {
                queue.removeNextRecord();
            }
        }

        // Queue is removed from the memory.
        // Check if file with data was deleted.
        assert(! fs::exists(fs::path(queuePath)));

        // Clearing
        if (fs::exists(fs::path(queuePath))) {
            fs::remove(fs::path(queuePath));
        }
    }
};
#include "OutgoingMessagesQueue.h"

OutgoingMessagesQueue::OutgoingMessagesQueue(NodeUUID *nodeUUID) {
    const string currentQueueUUID = boost::lexical_cast<string>(
        static_cast<uuid>(*nodeUUID));

    mFileQueue = new FileBackedMessagesQueue(currentQueueUUID);
}

void OutgoingMessagesQueue::enqueue(shared_ptr<Message> message) {
    mFileQueue->enqueue((*message).serialize());
}

// Initializes file backed cache for the outgoing messages.
// "filename" specifies the name of the cache.
// By default, it should be set to the UUID of the destination node,
// to prevent collision with names of caches of other nodes.
//
// If "autoTruncate" is set to "true" - then the cache will be automatically truncated
// to remove obsolete records.
// Internal logic of this class uses one more instance of this class,
// for the truncating purposes, and to prevent recursion that copy is initialised with
// "autoTruncate" = "false".
// It is recommended to leave "autoTruncate" enabled to prevent caches from growing.
FileBackedMessagesQueue::FileBackedMessagesQueue(const string &filename, bool autoTruncate) :
    mCacheFilename(filename), mAutoTruncate(autoTruncate), mWritesDone(0) {

    if (filename.empty()) {
        throw ValueError("Filename can't be empty!");
    }

    if (fileCacheExists())
        bindToExistingFileCache();
    else
        createEmptyFileCache();
}

FileBackedMessagesQueue::~FileBackedMessagesQueue() {
    if (mFileCacheDescriptor == nullptr) {
        // File descriptor may be closed into the truncateFileCache() method.
        // In this case - this instance is only the temporary instance,
        // and no additional clearing is needed for it.
        return;
    }

    // If file cache doesn't contains records - it may be removed from the FS.
    auto record = nextRecord();
    if (!record.first) {
        fclose(mFileCacheDescriptor);
        fs::remove(cacheFilePath());

    } else {
        // Try to truncate file on exit to prevent inefficient FS usage.
        if (mAutoTruncate)
            truncateFileCache();
    }
}

const bool FileBackedMessagesQueue::fileCacheExists() const {
    return fs::exists(fs::path(cacheFilePath()));
}

void FileBackedMessagesQueue::createEmptyFileCache() {
    ensureDirectoriesHierarchyIsPresent();

    mFileCacheDescriptor = fopen(cacheFilePath().c_str(), "w+");
    if (mFileCacheDescriptor == nullptr) {
        throw IOError("Can't create disk file cache!");
    }

    writeEmptyV1Header();
    mCacheIsReadOnly = false;
}

// Returns <true, some SerialisedMessage object> pair in case when next message is present
// and was successfully read. Otherwise - returns <false, null> pair.
const std::pair<bool, shared_ptr<SerialisedMessage>> FileBackedMessagesQueue::nextRecord() {
    if (mCacheIsReadOnly) {
        return std::pair<bool, shared_ptr<SerialisedMessage>>(
            false, shared_ptr<SerialisedMessage>(nullptr));
    }

    // Read next record size, that is stored into the header.
    seekToTheNextRecord();

    uint64_t recordSize = 0;
    auto recordsRead = fread(&recordSize, sizeof(recordSize), 1, mFileCacheDescriptor);
    if (recordsRead != 1) {
        // There are no any records in the cache.
        return std::pair<bool, shared_ptr<SerialisedMessage>>(
            false, shared_ptr<SerialisedMessage>(nullptr));
    }

    uint8_t *buffer = (uint8_t *) malloc(recordSize);
    if (buffer == nullptr) {
        throw MemoryError("Can't allocate memory for the next record buffer.");
    }

    recordsRead = fread(buffer, recordSize, 1, mFileCacheDescriptor);
    if (recordsRead != 1) {
        free(buffer);
        throw IOError("Can't read next record.");
    }

    // NOTE: SerialisedMessage will became controlling the life cycle of the "buffer".
    SerialisedMessage *message = new SerialisedMessage(buffer, recordSize);
    return std::pair<bool, shared_ptr<SerialisedMessage>>(
        true, shared_ptr<SerialisedMessage>(message));
}

void FileBackedMessagesQueue::enqueue(const shared_ptr<SerialisedMessage> &message) {
    if (mCacheIsReadOnly) {
        throw IOError("Attempt to modify read only file.");
    }

    // Ensure the file will be written at the end.
    seekToTheEnd();

    // Writing record size header.
    // This header describes how long the message is (in bytes),
    // so the parser will know how many bytes from this header should be read
    // when the message would be requested.
    const uint64_t messageSize = (*message).bytesCount();
    auto recordsWritten = fwrite(&messageSize, sizeof(messageSize), 1, mFileCacheDescriptor);
    if (recordsWritten != 1) {
        makeFileCacheReadOnly();
        throw IOError("Can't write message size header. "
                          "Write failed! "
                          "Queue file should be repaired. "
                          "No more messages should be written into this file.");
    }

    // Write message itself.
    recordsWritten = fwrite((*message).bytes(), messageSize, 1, mFileCacheDescriptor);
    if (recordsWritten != 1) {
        makeFileCacheReadOnly();
        throw IOError("Can't write message. "
                          "Write failed! "
                          "Queue file should be repaired. "
                          "No more messages would be written into this file.");
    }

    // Flush buffers to prevent data lost on crash.
    // After this line - the data should go into the OS's internal buffers.
    fflush(mFileCacheDescriptor);
    ++mWritesDone;

    if (mAutoTruncate && mWritesDone > 20) {
        mWritesDone = 0;

        // If file size is relatively big enough - try to truncate the file.
        if (ftell(mFileCacheDescriptor) > 2 * 1024) {
            truncateFileCache();
        }
    }
}

// Outgoing message may be not delivered after first attempt.
// So it may be re-sent several times.
// So, it should not be removed from the queue after first send attempt.
// Instead of that, message should be deleted after delivery confirmation was received.
//
// This method removes next message from the queue.
void FileBackedMessagesQueue::removeNextRecord() {
    if (mCacheIsReadOnly) {
        throw IOError("Attempt to modify read only file.");
    }

    seekToTheNextRecord();

    // Read record size.
    uint64_t recordSize = 0;
    auto recordsRead = fread(&recordSize, sizeof(recordSize), 1, mFileCacheDescriptor);
    if (recordsRead != 1) {
        throw IOError("Can't read next record index.");
    }

    // Fill the record with zeroes.
    // Record size (8 bytes) will remain in file,
    // for the ability to restore file structure in case of possible crash.
    //
    // Record may be of various size, so it should be zeroed byte-after-byte (uint8_t).
    uint8_t zero = 0;
    auto recordsWritten = fwrite(&zero, sizeof(zero), recordSize, mFileCacheDescriptor);
    if (recordsWritten != recordSize) {
        makeFileCacheReadOnly();
        throw IOError("Can't fill previous record with zeroes.");
    }

    // Read one more byte to perform EOF (if reached).
    fread(&zero, 1, 1, mFileCacheDescriptor);

    // If end of file is reached - than the file should be truncated.
    // Otherwise - the next record index should be updated.
    if (feof(mFileCacheDescriptor) != 0) {
        fclose(mFileCacheDescriptor);
        createEmptyFileCache();

    } else {
        auto currentPos = ftell(mFileCacheDescriptor) - 1; // - one read for EOF detecting.
        if (currentPos < 0) {
            // ftell returned error
            makeFileCacheReadOnly();
            throw IOError("Can't write index to the next record.");
        }

        // Seek back to the index.
        seekToTheHeaderNextRecordIndex();

        // Update the header with position of the next record.
        uint64_t nextRecordIndex = (uint64_t) currentPos;
        recordsWritten = fwrite(&nextRecordIndex, sizeof(nextRecordIndex), 1, mFileCacheDescriptor);
        if (recordsWritten != 1) {
            makeFileCacheReadOnly();
            throw IOError("Can't write index to the next record.");
        }
    }
}

// Sets file to readonly state.
void FileBackedMessagesQueue::makeFileCacheReadOnly() {
    // Set readonly flag in the file to prevent reading of the file on the next open attempt.
    uint16_t readOnlyState = 1;
    fseek(mFileCacheDescriptor, sizeof(uint16_t), 0);
    fwrite(&readOnlyState, sizeof(readOnlyState), 1, mFileCacheDescriptor);

    // Set readonly flag in the memory to prevent read attempts.
    mCacheIsReadOnly = true;
}

void FileBackedMessagesQueue::bindToExistingFileCache() {
    // Opens file for append only,
    // but with possibility to read from the beginning.
    mFileCacheDescriptor = fopen(cacheFilePath().c_str(), "rb+");
    if (mFileCacheDescriptor == nullptr) {
        throw IOError("Can't open cache file.");
    }

    // Ensure reading from the beginning.
    rewind(mFileCacheDescriptor);

    // Check file version.
    uint16_t version;
    fread(&version, sizeof(version), 1, mFileCacheDescriptor);
    if (version != 1) {
        throw IOError("Unexpected file version occurred. Can't proceed.");
    }

    // ...
    // Other versions processing goes here
    // ...

    // Check if file is not readonly.
    uint16_t state;
    fread(&state, sizeof(state), 1, mFileCacheDescriptor);
    mCacheIsReadOnly = (state == 1);

    if (state != 0 && state != 1) {
        throw IOError("Invalid file state occurred. Can't proceed.");
    }
}

void FileBackedMessagesQueue::seekToTheEnd() {
    fseek(mFileCacheDescriptor, 0, SEEK_END);
}

void FileBackedMessagesQueue::seekToTheNextRecord() {
    seekToTheHeaderNextRecordIndex();

    uint64_t nextRecordIndex = 0;
    auto recordsRead = fread(&nextRecordIndex, sizeof(nextRecordIndex), 1, mFileCacheDescriptor);
    if (recordsRead != 1) {
        throw IOError("Can't read next record index.");
    }

    // WARN: seek will proceed even if file doesn't contains any records.
    // The only way to prevent seeking outside of file - ois to measure its real size,
    // but it would be very inefficient.
    //
    // Record presence should be checked in read record method.
    fseek(mFileCacheDescriptor, nextRecordIndex, SEEK_SET);
}

void FileBackedMessagesQueue::seekToTheHeaderNextRecordIndex() {
    fseek(mFileCacheDescriptor, 2 * sizeof(uint16_t), SEEK_SET);
}

void FileBackedMessagesQueue::writeEmptyV1Header() {
    fseek(mFileCacheDescriptor, 0, SEEK_SET);

    uint16_t version = 1;
    auto recordsWritten = fwrite(&version, sizeof(version), 1, mFileCacheDescriptor);
    if (recordsWritten != 1) {
        throw IOError("Can't write file header.");
    }

    // State (2B)
    // 0 - file is correct.
    // 1 - file is read only.
    uint16_t stateCorrect = 0;
    recordsWritten = fwrite(&stateCorrect, sizeof(stateCorrect), 1, mFileCacheDescriptor);
    if (recordsWritten != 1) {
        throw IOError("Can't write file header.");
    }

    // Next record index (4B)
    uint64_t nullRecordIndex = sizeof(version) + sizeof(stateCorrect) + sizeof(uint64_t);
    recordsWritten = fwrite(&nullRecordIndex, sizeof(nullRecordIndex), 1, mFileCacheDescriptor);
    if (recordsWritten != 1) {
        throw IOError("Can't write zeroed next record index.");
    }
}

void FileBackedMessagesQueue::ensureDirectoriesHierarchyIsPresent() {
    fs::path cachePath(kCacheDirPath);
    if (!fs::exists(cachePath)) {
        fs::create_directories(cachePath);
    }
}

const string FileBackedMessagesQueue::cacheFilePath() const {
    return string(kCacheDirPath) + string(mCacheFilename) + string(kCacheFileExtension);
}

void FileBackedMessagesQueue::truncateFileCache() {
    seekToTheHeaderNextRecordIndex();

    uint64_t nextRecordIndex = 0;
    auto recordsRead = fread(&nextRecordIndex, sizeof(nextRecordIndex), 1, mFileCacheDescriptor);
    if (recordsRead != 1) {
        throw IOError("Can't read next record index.");
    }

    if (nextRecordIndex != 12) { // first record in the file
        // Truncating is possible.
        auto otherInstance = new FileBackedMessagesQueue(
            mCacheFilename + string("_tmp_copy"), false); // false prevents truncate recursion.

        for (;;) {
            auto record = nextRecord();
            if (record.first) {
                otherInstance->enqueue(record.second);
                removeNextRecord();

            } else {
                break;
            }
        }
        fclose(otherInstance->mFileCacheDescriptor);
        otherInstance->mFileCacheDescriptor = nullptr;

        fclose(mFileCacheDescriptor);
        mFileCacheDescriptor = nullptr;

        // Swap files
        fs::remove(fs::path(cacheFilePath()));
        fs::rename(fs::path(otherInstance->cacheFilePath()), fs::path(cacheFilePath()));

        bindToExistingFileCache();
        delete otherInstance;
    }
}

#ifndef GEO_NETWORK_CLIENT_OUTGOINGMESSAGESQUEUE_H
#define GEO_NETWORK_CLIENT_OUTGOINGMESSAGESQUEUE_H

#include "../messages/Message.h"
#include "../../common/NodeUUID.h"

#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/MemoryError.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <memory>
#include <fstream>
#include <stdlib.h>
#include <malloc.h>


using namespace std;
namespace fs = boost::filesystem;

class FileBackedMessagesQueue {
    friend class FileBackedMessagesQueueTests;

public:
    explicit FileBackedMessagesQueue(const std::string &filename, bool autoTruncate=true);
    ~FileBackedMessagesQueue();

    void enqueue(const std::shared_ptr<SerialisedMessage> &message);
    const std::pair<bool, std::shared_ptr<SerialisedMessage>> nextRecord();
    void removeNextRecord();

private:
    static constexpr const char *kCacheDirPath = "network/outgoing/messages_cache/";
    static constexpr const char *kCacheFileExtension = ".omc"; // Outgoing messages cache

private:
    inline const string cacheFilePath() const;
    inline const bool fileCacheExists() const;

    void bindToExistingFileCache();
    void createEmptyFileCache();
    void truncateFileCache();
    void ensureDirectoriesHierarchyIsPresent();
    void writeEmptyV1Header();
    void makeFileCacheReadOnly();

    inline void seekToTheEnd();
    inline void seekToTheNextRecord();
    inline void seekToTheHeaderNextRecordIndex();

private:
    std::string mCacheFilename;
    bool mCacheIsReadOnly;
    bool mAutoTruncate;
    uint8_t mWritesDone;
    FILE *mFileCacheDescriptor;
};


class OutgoingMessagesQueue {
public:
    OutgoingMessagesQueue(NodeUUID *nodeUUID);

    void enqueue(std::shared_ptr<Message> message);

private:
    FileBackedMessagesQueue *mFileQueue;
};
#endif //GEO_NETWORK_CLIENT_OUTGOINGMESSAGESQUEUE_H

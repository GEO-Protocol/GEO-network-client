#ifndef GEO_NETWORK_CLIENT_COMMUNICATORMESSAGESQUEUEHANDLER_H
#define GEO_NETWORK_CLIENT_COMMUNICATORMESSAGESQUEUEHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/Types.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/memory/MemoryUtils.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <tuple>

class CommunicatorMessagesQueueHandler {

public:
    CommunicatorMessagesQueueHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    // todo : uint16_t should change on typedef in Message.hpp
    void saveRecord(
        const NodeUUID &contractorUUID,
        const TransactionUUID &transactionUUID,
        const uint16_t messageType,
        BytesShared message,
        size_t messageBytesCount);

    vector<tuple<const NodeUUID, BytesShared, uint16_t>> allMessages();

    void deleteRecord(
        const NodeUUID &contractorUUID,
        const TransactionUUID &transactionUUID);

    void deleteRecord(
        const NodeUUID &contractorUUID,
        uint16_t messageType);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATORMESSAGESQUEUEHANDLER_H

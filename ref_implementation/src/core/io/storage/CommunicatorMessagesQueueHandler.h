/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_COMMUNICATORMESSAGESQUEUEHANDLER_H
#define GEO_NETWORK_CLIENT_COMMUNICATORMESSAGESQUEUEHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/Types.h"
#include "../../network/messages/Message.hpp"
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

    void saveRecord(
        const NodeUUID &contractorUUID,
        const TransactionUUID &transactionUUID,
        const Message::SerializedType messageType,
        BytesShared message,
        size_t messageBytesCount);

    vector<tuple<const NodeUUID, BytesShared, Message::SerializedType>> allMessages();

    void deleteRecord(
        const NodeUUID &contractorUUID,
        const Message::SerializedType messageType);

    void deleteRecord(
        const NodeUUID &contractorUUID,
        const TransactionUUID &transactionUUID);

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

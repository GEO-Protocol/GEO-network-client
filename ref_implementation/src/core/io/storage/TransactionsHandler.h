/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H

#include "../../logger/Logger.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>

class TransactionsHandler {

public:
    TransactionsHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveRecord(
        const TransactionUUID &transactionUUID,
        BytesShared transaction,
        size_t transactionBytesCount);

    pair<BytesShared, size_t> getTransaction(
        const TransactionUUID &transactionUUID);

    void deleteRecord(
        const TransactionUUID &transactionUUID);

    vector<pair<BytesShared, size_t>> allTransactions();

    const string tableName() const;

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H

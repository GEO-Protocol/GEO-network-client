#ifndef GEO_NETWORK_CLIENT_TRANSACTIONHANDLER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONHANDLER_H

#include "../../logger/Logger.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"

#include "../../../libs/sqlite3/sqlite3.h"

class TransactionHandler {

public:

    TransactionHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger *logger);

    void saveRecord(
        const TransactionUUID &transactionUUID,
        BytesShared transaction,
        size_t transactionBytesCount);

    pair<BytesShared, size_t> getTransaction(
        const TransactionUUID &transactionUUID);

    void deleteRecord(
        const TransactionUUID &transactionUUID);

    bool commit();

    void rollBack();

private:

    void prepareInserted();

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger *mLog;
    bool isTransactionBegin;

};


#endif //GEO_NETWORK_CLIENT_TRANSACTIONHANDLER_H

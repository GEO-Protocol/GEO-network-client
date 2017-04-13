#ifndef GEO_NETWORK_CLIENT_PAYMENTOPERATIONSTATEHANDLER_H
#define GEO_NETWORK_CLIENT_PAYMENTOPERATIONSTATEHANDLER_H

#include "../../logger/Logger.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"

#include "../../../libs/sqlite3/sqlite3.h"

class PaymentOperationStateHandler {

public:

    PaymentOperationStateHandler(
        sqlite3 *db,
        const string &tableName,
        Logger *logger);

    void saveRecord(
        const TransactionUUID &transactionUUID,
        BytesShared state,
        size_t stateBytesCount);

    pair<BytesShared, size_t> getState(
        const TransactionUUID &transactionUUID);

    void deleteRecord(
        const TransactionUUID &transactionUUID);

    void commit();

    void rollBack();

private:

    void prepareInserted();

    void insert(
        const TransactionUUID &transactionUUID,
        BytesShared state,
        size_t stateBytesCount);

    void update(
        const TransactionUUID &transactionUUID,
        BytesShared state,
        size_t stateBytesCount);

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase;
    string mTableName;
    sqlite3_stmt *stmt;
    Logger *mLog;
    bool isTransactionBegin;

};


#endif //GEO_NETWORK_CLIENT_PAYMENTOPERATIONSTATEHANDLER_H

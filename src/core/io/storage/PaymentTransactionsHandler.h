#ifndef GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONSHANDLER_H

#include "../../logger/Logger.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/Types.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/NotFoundError.h"

#include "../../../libs/sqlite3/sqlite3.h"

class PaymentTransactionsHandler {

public:
    PaymentTransactionsHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveRecord(
        const TransactionUUID &transactionUUID,
        BlockNumber maximalClaimingBlockNumber);

    void updateTransactionState(
        const TransactionUUID &transactionUUID,
        int observingTransactionState);

    vector<pair<TransactionUUID, BlockNumber>> transactionsWithUncertainObservingState();

    bool isTransactionCommitted(
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


#endif //GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONSHANDLER_H

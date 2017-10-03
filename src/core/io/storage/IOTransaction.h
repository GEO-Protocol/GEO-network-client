#ifndef GEO_NETWORK_CLIENT_IOTRANSACTION_H
#define GEO_NETWORK_CLIENT_IOTRANSACTION_H

#include "../../common/Types.h"
#include "TrustLineHandler.h"
#include "HistoryStorage.h"
#include "PaymentOperationStateHandler.h"
#include "TransactionsHandler.h"

#include "../../../libs/sqlite3/sqlite3.h"

class IOTransaction {

public:
    typedef shared_ptr<IOTransaction> Shared;

public:
    IOTransaction(
        sqlite3 *dbConnection,
        TrustLineHandler *trustLinesHandler,
        HistoryStorage *historyStorage,
        PaymentOperationStateHandler *paymentOperationStorage,
        TransactionsHandler *transactionHandler,
        Logger &logger);

    ~IOTransaction();

    TrustLineHandler *trustLinesHandler();

    HistoryStorage *historyStorage();

    PaymentOperationStateHandler *paymentOperationStateHandler();

    TransactionsHandler *transactionHandler();

    void rollback();

private:

    void commit();

    void beginTransactionQuery();

    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDBConnection;
    TrustLineHandler *mTrustLineHandler;
    HistoryStorage *mHistoryStorage;
    PaymentOperationStateHandler *mPaymentOperationStateHandler;
    TransactionsHandler *mTransactionHandler;
    bool mIsTransactionBegin;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_IOTRANSACTION_H

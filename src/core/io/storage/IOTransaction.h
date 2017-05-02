#ifndef GEO_NETWORK_CLIENT_IOTRANSACTION_H
#define GEO_NETWORK_CLIENT_IOTRANSACTION_H

#include "../../common/Types.h"
#include "RoutingTablesHandler.h"
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
        RoutingTablesHandler *routingTablesHandler,
        TrustLineHandler *trustLineHandler,
        HistoryStorage *historyStorage,
        PaymentOperationStateHandler *paymentOperationStorage,
        TransactionsHandler *transactionHandler,
        Logger *logger);

    ~IOTransaction();

    RoutingTablesHandler* routingTablesHandler();

    TrustLineHandler *trustLineHandler();

    HistoryStorage *historyStorage();

    PaymentOperationStateHandler *paymentOperationStateHandler();

    TransactionsHandler *transactionHandler();

    void commit();

    void rollback();

private:
    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    sqlite3 *mDBConnection;
    RoutingTablesHandler *mRoutingTablesHandler;
    TrustLineHandler *mTrustLineHandler;
    HistoryStorage *mHistoryStorage;
    PaymentOperationStateHandler *mPaymentOperationStateHandler;
    TransactionsHandler *mTransactionHandler;
    bool mIsTransactionBegin;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_IOTRANSACTION_H

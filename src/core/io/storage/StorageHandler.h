#ifndef GEO_NETWORK_CLIENT_STORAGEHANDLER_H
#define GEO_NETWORK_CLIENT_STORAGEHANDLER_H

#include "../../logger/Logger.h"
#include "RoutingTablesHandler.h"
#include "RoutingTableHandler.h"
#include "TrustLineHandler.h"
#include "PaymentOperationStateHandler.h"
#include "TransactionHandler.h"
#include "HistoryStorage.h"
#include "../../common/exceptions/IOError.h"
#include "../../../libs/sqlite3/sqlite3.h"
#include "IOTransaction.h"

#include <boost/filesystem.hpp>
#include <vector>

namespace fs = boost::filesystem;

class StorageHandler {

public:
    StorageHandler(
        const string &directory,
        const string &dataBaseName,
        Logger *logger);

    ~StorageHandler();

    IOTransaction::Shared beginTransaction();

    // TODO: need discussion (cycles)
    RoutingTablesHandler* routingTablesHandler();

private:
    static void checkDirectory(
        const string &directory);

    static sqlite3* connection(
        const string &dataBaseName,
        const string &directory);

    void beginTransactionQuery();

    LoggerStream info() const;

    const string logHeader() const;

private:
    const string kRT2TableName = "RT2";
    const string kRT3TableName = "RT3";
    const string kTrustLineTableName = "trust_lines";
    const string kPaymentOperationStateTableName = "payment_operation_state";
    const string kTransactionTableName = "transactions";
    const string kHistoryTableName = "history";

private:
    static sqlite3 *mDBConnection;

private:
    Logger *mLog;
    RoutingTablesHandler mRoutingTablesHandler;
    TrustLineHandler mTrustLineHandler;
    PaymentOperationStateHandler mPaymentOperationStateHandler;
    TransactionHandler mTransactionHandler;
    HistoryStorage mHistoryStorage;

    string mDirectory;
    string mDataBaseName;
};


#endif //GEO_NETWORK_CLIENT_STORAGEHANDLER_H

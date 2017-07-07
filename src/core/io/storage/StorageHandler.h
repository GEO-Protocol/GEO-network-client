#ifndef GEO_NETWORK_CLIENT_STORAGEHANDLER_H
#define GEO_NETWORK_CLIENT_STORAGEHANDLER_H

#include "../../logger/Logger.h"
#include "RoutingTablesHandler.h"
#include "RoutingTableHandler.h"
#include "TrustLineHandler.h"
#include "PaymentOperationStateHandler.h"
#include "TransactionsHandler.h"
#include "MigrationsHandler.h"
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
        Logger &logger);

    ~StorageHandler();

    IOTransaction::Shared beginTransaction();

    RoutingTablesHandler* routingTablesHandler();

    void backupStorageHandler();

    int applyMigrations();

private:
    static void checkDirectory(
        const string &directory);

    static sqlite3* connection(
        const string &dataBaseName,
        const string &directory);

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    const string kRT2TableName = "RT2";
    const string kRT3TableName = "RT3";
    const string kTrustLineTableName = "trust_lines";
    const string kPaymentOperationStateTableName = "payment_operation_state";
    const string kTransactionTableName = "transactions";
    const string kHistoryMainTableName = "history";
    const string kHistoryAdditionalTableName = "history_additional";
    const string kMigrationTableName = "migrations";

private:
    static sqlite3 *mDBConnection;

private:
    Logger &mLog;
    RoutingTablesHandler mRoutingTablesHandler;
    TrustLineHandler mTrustLineHandler;
    PaymentOperationStateHandler mPaymentOperationStateHandler;
    TransactionsHandler mTransactionHandler;
    HistoryStorage mHistoryStorage;
    string mDirectory;
    string mDataBaseName;
};


#endif //GEO_NETWORK_CLIENT_STORAGEHANDLER_H

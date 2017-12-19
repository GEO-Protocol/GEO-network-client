#ifndef GEO_NETWORK_CLIENT_STORAGEHANDLER_H
#define GEO_NETWORK_CLIENT_STORAGEHANDLER_H

#include "../../logger/Logger.h"
#include "TrustLineHandler.h"
#include "PaymentOperationStateHandler.h"
#include "TransactionsHandler.h"
#include "MigrationsHandler.h"
#include "HistoryStorage.h"
#include "BlackListHandler.h"
#include "NodeFeaturesHandler.h"
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

    int applyMigrations(const NodeUUID &nodeUUID);

private:
    static void checkDirectory(
        const string &directory);

    static sqlite3* connection(
        const string &dataBaseName,
        const string &directory);

    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    const string kTrustLineTableName = "trust_lines";
    const string kPaymentOperationStateTableName = "payment_operation_state";
    const string kTransactionTableName = "transactions";
    const string kHistoryMainTableName = "history";
    const string kHistoryAdditionalTableName = "history_additional";
    const string kMigrationTableName = "migrations";
    const string kNodeFeaturesTableName = "node_features";
    const string kBlackListTableName = "blacklist";
//    const string kOutgoingNetworkMessagesQueue = "network_outgoing_messages";

private:
    static sqlite3 *mDBConnection;

private:
    Logger &mLog;
    TrustLineHandler mTrustLineHandler;
    PaymentOperationStateHandler mPaymentOperationStateHandler;
    TransactionsHandler mTransactionHandler;
    HistoryStorage mHistoryStorage;
    BlackListHandler mBlackListHandler;
    NodeFeaturesHandler mNodeFeaturesHandler;
    string mDirectory;
    string mDataBaseName;
};


#endif //GEO_NETWORK_CLIENT_STORAGEHANDLER_H

#include "StorageHandler.h"

sqlite3 *StorageHandler::mDBConnection = nullptr;

StorageHandler::StorageHandler(
    const string &directory,
    const string &dataBaseName,
    Logger *logger):

    mDirectory(directory),
    mDataBaseName(dataBaseName),
    mRoutingTablesHandler(connection(dataBaseName, directory), kRT2TableName, kRT3TableName, logger),
    mTrustLineHandler(connection(dataBaseName, directory), kTrustLineTableName, logger),
    mPaymentOperationStateHandler(connection(dataBaseName, directory), kPaymentOperationStateTableName, logger),
    mTransactionHandler(connection(dataBaseName, directory), kTransactionTableName, logger),
    mLog(logger) {

    sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
}

StorageHandler::~StorageHandler() {

    closeConnections();
}

RoutingTablesHandler* StorageHandler::routingTablesHandler() {

    return &mRoutingTablesHandler;
}

TrustLineHandler* StorageHandler::trustLineHandler() {

    return &mTrustLineHandler;
}

PaymentOperationStateHandler *StorageHandler::paymentOperationStateHandler() {

    return &mPaymentOperationStateHandler;
}

TransactionHandler *StorageHandler::transactionHandler() {

    return &mTransactionHandler;
}

void StorageHandler::closeConnections() {

    mRoutingTablesHandler.closeConnections();
    mTrustLineHandler.closeConnection();
    mPaymentOperationStateHandler.closeConnection();
    mTransactionHandler.closeConnection();
}

void StorageHandler::checkDirectory(
    const string &directory) {

    if (!fs::is_directory(fs::path(directory))){
        fs::create_directories(
            fs::path(directory));
    }
}

sqlite3* StorageHandler::connection(
    const string &dataBaseName,
    const string &directory) {

    checkDirectory(directory);

    if (mDBConnection != nullptr)
        return mDBConnection;

    string dataBasePath = directory + "/" + dataBaseName;
    int rc = sqlite3_open_v2(dataBasePath.c_str(), &mDBConnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc == SQLITE_OK) {
    } else {
        throw IOError("StorageHandler::connection "
                          "Can't open database " + dataBaseName);
    }
    return mDBConnection;
}


LoggerStream StorageHandler::info() const {
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

const string StorageHandler::logHeader() const {
    stringstream s;
    s << "[StorageHandler]";
    return s.str();
}

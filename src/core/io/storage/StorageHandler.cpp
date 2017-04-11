#include "StorageHandler.h"

sqlite3 *StorageHandler::mDataBase = nullptr;

StorageHandler::StorageHandler(
    const string &directory,
    const string &dataBaseName,
    Logger *logger):

    mDirectory(directory),
    mDataBaseName(dataBaseName),
    /*mRoutingTablesHandler(buildDataBasePath(directory, dataBaseName), kRT2TableName, kRT3TableName, logger),
    mTrustLineHandler(buildDataBasePath(directory, dataBaseName), kTrustLineTableName, logger),
    mPaymentOperationStateHandler(buildDataBasePath(directory, dataBaseName), kPaymentOperationStateTableName, logger),*/
    mRoutingTablesHandler(connection(dataBaseName, directory), mDataBaseName, kRT2TableName, kRT3TableName, logger),
    mTrustLineHandler(connection(dataBaseName, directory), mDataBaseName, kTrustLineTableName, logger),
    mPaymentOperationStateHandler(connection(dataBaseName, directory), mDataBaseName, kPaymentOperationStateTableName, logger),
    mLog(logger) {

}

StorageHandler::~StorageHandler() {

    closeConnections();
}

sqlite3* StorageHandler::connection(
    const string &dataBaseName,
    const string &directory) {
    checkDirectory(directory);
    if (mDataBase != nullptr)
        return mDataBase;
    string dataBasePath = directory + "/" + dataBaseName;
    int rc = sqlite3_open_v2(dataBasePath.c_str(), &mDataBase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc == SQLITE_OK) {
    } else {
        throw IOError("StorageHandler::connection "
                          "Can't open database " + dataBaseName);
    }
    return mDataBase;
}

void StorageHandler::checkDirectory(const string &directory) {
    if (!fs::is_directory(fs::path(directory))){
        fs::create_directories(
            fs::path(directory));
    }
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

void StorageHandler::closeConnections() {

    mRoutingTablesHandler.closeConnections();
    mTrustLineHandler.closeConnection();
    mPaymentOperationStateHandler.closeConnection();
}

string StorageHandler::buildDataBasePath(
    const string &directory,
    const string &dataBaseName) {

    string dataBasePath = directory + "/" + dataBaseName;
    if (!fs::is_directory(fs::path(directory))){
        fs::create_directories(
            fs::path(directory));
    }
    return dataBasePath;
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

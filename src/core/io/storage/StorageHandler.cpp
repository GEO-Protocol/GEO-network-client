#include "StorageHandler.h"

sqlite3 *StorageHandler::mDataBase = nullptr;

StorageHandler::StorageHandler(
    const string &directory,
    const string &dataBaseName,
    Logger *logger):

    mDirectory(directory),
    mDataBaseName(dataBaseName),
    mDataBasePath(mDirectory + "/" + dataBaseName),
    mRoutingTablesHandler(connection(dataBaseName, directory), kRT2TableName, kRT3TableName, logger),
    mTrustLineHandler(connection(dataBaseName, directory), kTrustLineTableName, logger),
    mPaymentOperationStateHandler(connection(dataBaseName, directory), kPaymentOperationStateTableName, logger),
    mLog(logger) {

#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "Opened database successfully";
#endif
}

StorageHandler::~StorageHandler() {

    closeConnection();
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

RoutingTablesHandler* StorageHandler::routingTablesHandler() {

    return &mRoutingTablesHandler;
}

TrustLineHandler* StorageHandler::trustLineHandler() {

    return &mTrustLineHandler;
}

PaymentOperationStateHandler *StorageHandler::paymentOperationStateHandler() {

    return &mPaymentOperationStateHandler;
}

void StorageHandler::closeConnection() {
    sqlite3_close_v2(mDataBase);
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "Connection closed";
#endif
}

void StorageHandler::checkDirectory(const string &directory) {

    if (!fs::is_directory(fs::path(directory))){
        fs::create_directories(
            fs::path(directory));
    }
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

#include "StorageHandler.h"

sqlite3 *StorageHandler::mDBConnection = nullptr;

StorageHandler::StorageHandler(
    const string &directory,
    const string &dataBaseName,
    Logger &logger):

    mDirectory(directory),
    mDataBaseName(dataBaseName),
    mRoutingTablesHandler(connection(dataBaseName, directory), kRT2TableName, kRT3TableName, logger),
    mTrustLineHandler(connection(dataBaseName, directory), kTrustLineTableName, logger),
    mPaymentOperationStateHandler(connection(dataBaseName, directory), kPaymentOperationStateTableName, logger),
    mTransactionHandler(connection(dataBaseName, directory), kTransactionTableName, logger),
    mHistoryStorage(connection(dataBaseName, directory), kHistoryMainTableName, kHistoryAdditionalTableName, logger),
    mLog(logger)
{
    sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
}

StorageHandler::~StorageHandler()
{
    if (mDBConnection != nullptr) {
        sqlite3_close_v2(mDBConnection);
    }
}

void StorageHandler::checkDirectory(
    const string &directory)
{
    if (!fs::is_directory(fs::path(directory))){
        fs::create_directories(
            fs::path(directory));
    }
}

sqlite3* StorageHandler::connection(
    const string &dataBaseName,
    const string &directory)
{
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

IOTransaction::Shared StorageHandler::beginTransaction()
{

    return make_shared<IOTransaction>(
        mDBConnection,
        &mRoutingTablesHandler,
        &mTrustLineHandler,
        &mHistoryStorage,
        &mPaymentOperationStateHandler,
        &mTransactionHandler,
        mLog);
}

LoggerStream StorageHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream StorageHandler::error() const
{
    return mLog.error(logHeader());
}

const string StorageHandler::logHeader() const
{
    stringstream s;
    s << "[StorageHandler]";
    return s.str();
}

int StorageHandler::applyMigrations(const NodeUUID &nodeUUID) {
    auto ioTransaction = beginTransaction();
    auto migrationHandler = make_shared<MigrationsHandler>(
            mDBConnection,
            kMigrationTableName,
            nodeUUID,
            mLog);

    try {
        migrationHandler->applyMigrations(ioTransaction);
        return 0;

    } catch(const Exception &e) {
        mLog.error("") << e.what();
        ioTransaction->rollback();
        return -1;
    }
}
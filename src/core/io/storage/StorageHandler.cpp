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
    mHistoryStorage(connection(dataBaseName, directory), kHistoryTableName, logger),
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

RoutingTablesHandler* StorageHandler::routingTablesHandler()
{
    return &mRoutingTablesHandler;
}

IOTransaction::Shared StorageHandler::beginTransaction()
{
    beginTransactionQuery();
    return make_shared<IOTransaction>(
        mDBConnection,
        &mRoutingTablesHandler,
        &mTrustLineHandler,
        &mHistoryStorage,
        &mPaymentOperationStateHandler,
        &mTransactionHandler,
        mLog);
}

void StorageHandler::beginTransactionQuery()
{
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "beginTransactionQuery";
#endif
    string query = "BEGIN TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("StorageHandler::prepareInserted: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("StorageHandler::prepareInserted: Run query; sqlite error: " + rc);
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction begin";
#endif
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

void StorageHandler::backupStorageHandler()
{
    string destinationDBName = "storageDBBackup";
    string directory = "io";
    sqlite3_backup *sql3Backup;
    sqlite3 *destinationDB;
    string dataBasePath = directory + "/" + destinationDBName;
    int rc = sqlite3_open_v2(dataBasePath.c_str(), &destinationDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

    if (rc == SQLITE_OK) {
    } else {
        throw IOError("StorageHandler::connection "
                              "Can't open database " + destinationDBName);
    }

    sql3Backup = sqlite3_backup_init(
            destinationDB,
            "main",
            connection(mDataBaseName, mDirectory),
            "main");

    if (sql3Backup == NULL ){
        error() << sqlite3_errmsg(destinationDB);
        return;
    }

    if(sqlite3_backup_step(sql3Backup, -1) != SQLITE_DONE){
        error() << sqlite3_errmsg(destinationDB);
        return;
    }

    if(sqlite3_backup_finish(sql3Backup) != SQLITE_OK ){
        error() << sqlite3_errmsg(destinationDB);
        return;
    };

    info() << "Successfully create dump" << endl;
}

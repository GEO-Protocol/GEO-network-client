#include "StorageHandler.h"

sqlite3 *StorageHandler::mDBConnection = nullptr;

StorageHandler::StorageHandler(
    const string &directory,
    const string &dataBaseName,
    Logger &logger):

    mDirectory(directory),
    mDataBaseName(dataBaseName),
    mTrustLineHandler(connection(dataBaseName, directory), kTrustLineTableName, logger),
    mTransactionHandler(connection(dataBaseName, directory), kTransactionTableName, logger),
    mHistoryStorage(connection(dataBaseName, directory), kHistoryMainTableName, kHistoryAdditionalTableName, logger),
    mBlackListHandler(connection(dataBaseName, directory), kBlackListTableName, logger),
    mOwnKeysHandler(connection(dataBaseName, directory), kOwnKeysTableName, logger),
    mContractorKeysHandler(connection(dataBaseName, directory), kContractorKeysTableName, logger),
    mAuditHandler(connection(dataBaseName, directory), kAuditTableName, logger),
    mIncomingPaymentReceiptHandler(connection(dataBaseName, directory), kIncomingReceiptTableName, logger),
    mOutgoingPaymentReceiptHandler(connection(dataBaseName, directory), kOutgoingReceiptTableName, logger),
    mPaymentKeysHandler(connection(dataBaseName, directory), kPaymentKeysTableName, logger),
    mPaymentParticipantsVotesHandler(connection(dataBaseName, directory), kPaymentParticipantsVotesTableName, logger),
    mLog(logger)
{
    sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);

    sqlite3_stmt *stmt;
    string query = "PRAGMA foreign_keys = ON;";
    int rc = sqlite3_prepare_v2( mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("StorageHandler::enabling foreign keys: "
                              "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("StorageHandler::enabling foreign keys: "
                              "Run query; sqlite error: " + to_string(rc));
    }
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
        &mTrustLineHandler,
        &mHistoryStorage,
        &mTransactionHandler,
        &mBlackListHandler,
        &mOwnKeysHandler,
        &mContractorKeysHandler,
        &mAuditHandler,
        &mIncomingPaymentReceiptHandler,
        &mOutgoingPaymentReceiptHandler,
        &mPaymentKeysHandler,
        &mPaymentParticipantsVotesHandler,
        mLog);
}

LoggerStream StorageHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream StorageHandler::warning() const
{
    return mLog.warning(logHeader());
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
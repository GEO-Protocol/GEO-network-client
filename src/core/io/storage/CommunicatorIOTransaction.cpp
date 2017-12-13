#include "CommunicatorIOTransaction.h"

CommunicatorIOTransaction::CommunicatorIOTransaction(
    sqlite3 *dbConnection,
    CommunicatorMessagesQueueHandler *communicatorMessagesQueueHandler,
    Logger &logger) :

    mDBConnection(dbConnection),
    mCommunicatorMessagesQueueHandler(communicatorMessagesQueueHandler),
    mIsTransactionBegin(true),
    mLog(logger)
{
    beginTransactionQuery();
}

CommunicatorIOTransaction::~CommunicatorIOTransaction()
{
    commit();
}

CommunicatorMessagesQueueHandler* CommunicatorIOTransaction::communicatorMessagesQueueHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("CommunicatorIOTransaction::communicatorMessagesQueueHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mCommunicatorMessagesQueueHandler;
}

void CommunicatorIOTransaction::commit()
{
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "commit";
#endif
    if (!mIsTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction don't commit it was rollbacked";
#endif
        return;
    }
    string query = "COMMIT TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorIOTransaction::commit: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("CommunicatorIOTransaction::commit: Run query; sqlite error: " + to_string(rc));
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction commit";
#endif
}

void CommunicatorIOTransaction::rollback()
{
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "rollback";
#endif
    string query = "ROLLBACK TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorIOTransaction::rollback: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("CommunicatorIOTransaction::rollback: Run query; sqlite error: " + to_string(rc));
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "rollBack done";
#endif
    mIsTransactionBegin = false;
}

LoggerStream CommunicatorIOTransaction::info() const
{
    return mLog.info(logHeader());
}

LoggerStream CommunicatorIOTransaction::warning() const
{
    return mLog.warning(logHeader());
}

const string CommunicatorIOTransaction::logHeader() const
{
    stringstream s;
    s << "[CommunicatorIOTransaction]";
    return s.str();
}

void CommunicatorIOTransaction::beginTransactionQuery() {
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "beginTransactionQuery";
#endif
    string query = "BEGIN TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorIOTransaction::prepareInserted: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("CommunicatorIOTransaction::prepareInserted: Run query; sqlite error: " + to_string(rc));
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction begin";
#endif
}

#include "IOTransaction.h"

IOTransaction::IOTransaction(
    sqlite3 *dbConnection,
    TrustLineHandler *trustLineHandler,
    HistoryStorage *historyStorage,
    PaymentOperationStateHandler *paymentOperationStorage,
    TransactionsHandler *transactionHandler,
    NodeFeaturesHandler *nodeFeaturesHandler,
    Logger &logger) :

    mDBConnection(dbConnection),
    mTrustLineHandler(trustLineHandler),
    mHistoryStorage(historyStorage),
    mPaymentOperationStateHandler(paymentOperationStorage),
    mTransactionHandler(transactionHandler),
    mNodeFeaturesHandler(nodeFeaturesHandler),
    mIsTransactionBegin(true),
    mLog(logger)
{
    beginTransactionQuery();
}

IOTransaction::~IOTransaction()
{
    commit();
}

TrustLineHandler* IOTransaction::trustLinesHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::trustLineHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mTrustLineHandler;
}

HistoryStorage* IOTransaction::historyStorage()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::historyStorage: "
                          "transaction was rollback, it can't be use now");
    }
    return mHistoryStorage;
}

PaymentOperationStateHandler* IOTransaction::paymentOperationStateHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::paymentOperationStateHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mPaymentOperationStateHandler;
}

TransactionsHandler* IOTransaction::transactionHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::transactionHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mTransactionHandler;
}

NodeFeaturesHandler* IOTransaction::nodeFeaturesHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::nodeFeaturesHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mNodeFeaturesHandler;
}

void IOTransaction::commit()
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
        throw IOError("IOTransaction::commit: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("IOTransaction::commit: Run query; sqlite error: " + to_string(rc));
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction commit";
#endif
}

void IOTransaction::rollback()
{
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "rollback";
#endif
    string query = "ROLLBACK TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IOTransaction::rollback: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("IOTransaction::rollback: Run query; sqlite error: " + to_string(rc));
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "rollBack done";
#endif
    mIsTransactionBegin = false;
}

LoggerStream IOTransaction::info() const
{
    return mLog.info(logHeader());
}

LoggerStream IOTransaction::warning() const
{
    return mLog.warning(logHeader());
}

const string IOTransaction::logHeader() const
{
    stringstream s;
    s << "[IOTransaction]";
    return s.str();
}

void IOTransaction::beginTransactionQuery() {
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "beginTransactionQuery";
#endif
    string query = "BEGIN TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IOTransaction::prepareInserted: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("IOTransaction::prepareInserted: Run query; sqlite error: " + to_string(rc));
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction begin";
#endif
}

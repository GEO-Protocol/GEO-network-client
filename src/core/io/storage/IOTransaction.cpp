#include "IOTransaction.h"

IOTransaction::IOTransaction(
    sqlite3 *dbConnection,
    RoutingTablesHandler *routingTablesHandler,
    TrustLineHandler *trustLineHandler,
    HistoryStorage *historyStorage,
    PaymentOperationStateHandler *paymentOperationStorage,
    TransactionHandler *transactionHandler,
    Logger *logger) :

    mDBConnection(dbConnection),
    mRoutingTablesHandler(routingTablesHandler),
    mTrustLineHandler(trustLineHandler),
    mHistoryStorage(historyStorage),
    mPaymentOperationStateHandler(paymentOperationStorage),
    mTransactionHandler(transactionHandler),
    mLog(logger)
{}

IOTransaction::~IOTransaction() {
    commit();
}

RoutingTablesHandler* IOTransaction::routingTablesHandler() {

    return mRoutingTablesHandler;
}

TrustLineHandler* IOTransaction::trustLineHandler() {

    return mTrustLineHandler;
}

HistoryStorage* IOTransaction::historyStorage() {

    return mHistoryStorage;
}

PaymentOperationStateHandler* IOTransaction::paymentOperationStateHandler() {

    return mPaymentOperationStateHandler;
}

TransactionHandler* IOTransaction::transactionHandler() {

    return mTransactionHandler;
}

void IOTransaction::commit() {
    string query = "COMMIT TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IOTransaction::commit: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("IOTransaction::commit: Run query; sqlite error: " + rc);
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction commit";
#endif
}

void IOTransaction::rollback() {
    string query = "ROLLBACK TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IOTransaction::rollback: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("IOTransaction::rollback: Run query; sqlite error: " + rc);
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "rollBack done";
#endif
}

LoggerStream IOTransaction::info() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream IOTransaction::error() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string IOTransaction::logHeader() const
{
    stringstream s;
    s << "[IOTransaction]";
    return s.str();
}
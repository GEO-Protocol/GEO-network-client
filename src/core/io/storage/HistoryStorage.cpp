#include "HistoryStorage.h"

HistoryStorage::HistoryStorage(
    sqlite3 *dbConnection,
    const string &paymentsTableName,
    const string &trustLinesTableName,
    Logger *logger) :

    mDataBase(dbConnection),
    mPaymentsTableName(paymentsTableName),
    mTrustLinesTableName(trustLinesTableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mPaymentsTableName +
                   "(operation_uuid BLOB NOT NULL, "
                       "operation_timestamp INTEGER NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::creating payments table: Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::creating payments table: Run query");
    }

    query = "CREATE TABLE IF NOT EXISTS " + mTrustLinesTableName +
                   "(operation_uuid BLOB NOT NULL, "
                       "operation_timestamp INTEGER NOT NULL);";
    rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::creating trustlines table: Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::creating trustlines table: Run query");
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

LoggerStream HistoryStorage::info() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream HistoryStorage::error() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string HistoryStorage::logHeader() const
{
    stringstream s;
    s << "[HistoryStorage]";
    return s.str();
}

#include "PaymentTransactionsHandler.h"

PaymentTransactionsHandler::PaymentTransactionsHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger):

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (uuid BLOB NOT NULL, "
                   "maximal_claiming_block_number BLOB NOT NULL, "
                   "observing_state INTEGER NOT NULL, "
                   "recording_time INTEGER NOT NULL);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PaymentTransactionsHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_uuid_idx on " + mTableName + " (uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::creating index for TransactionUUID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PaymentTransactionsHandler::creating index for TransactionUUID: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void PaymentTransactionsHandler::saveRecord(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber)
{
    string query = "INSERT INTO " + mTableName + " (uuid, maximal_claiming_block_number, "
            "observing_state, recording_time) VALUES(?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::saveRecord: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::saveRecord: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, &maximalClaimingBlockNumber, sizeof(BlockNumber), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::saveRecord: "
                          "Bad binding of Maximal claiming block number; sqlite error: " + to_string(rc));
    }
    // todo : use constant instead 0
    rc = sqlite3_bind_int(stmt, 3, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::saveRecord: "
                          "Bad binding of Observing state; sqlite error: " + to_string(rc));
    }
    GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(utc_now());
    rc = sqlite3_bind_int64(stmt, 4, timestamp);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::saveRecord: "
                          "Bad binding of Timestamp; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("PaymentTransactionsHandler::saveRecord: "
                              "Run query; sqlite error: " + to_string(rc));
    }
}

void PaymentTransactionsHandler::updateTransactionState(
    const TransactionUUID &transactionUUID,
    int observingTransactionState)
{
    string query = "UPDATE " + mTableName +
                   " SET observing_state = ? WHERE uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::updateTransactionState: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, observingTransactionState);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::updateTransactionState: "
                          "Bad binding of Observing State; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, transactionUUID.data, TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::updateTransactionState: "
                          "Bad binding of UUID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare updating is completed successfully";
#endif
    } else {
        throw IOError("PaymentTransactionsHandler::updateTransactionState: "
                              "Run query; sqlite error: " + to_string(rc));
    }

    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were modified");
    }
}

vector<pair<TransactionUUID, BlockNumber>> PaymentTransactionsHandler::transactionsWithUncertainObservingState()
{
    vector<pair<TransactionUUID, BlockNumber>> result;
    sqlite3_stmt *stmt;

    // todo : use constant instead 0 in query
    string query = "SELECT uuid, maximal_claiming_block_number FROM "
                   + mTableName + " WHERE observing_state == 0";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::allTrustLinesByEquivalent: "
                              "Bad query; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        TransactionUUID transactionUUID((uint8_t *)sqlite3_column_blob(stmt, 0));

        auto blockNumberBytes = sqlite3_column_blob(stmt, 1);
        BlockNumber maximalClaimingBlockNumber;
        memcpy(
            &maximalClaimingBlockNumber,
            blockNumberBytes,
            sizeof(BlockNumber));

        result.emplace_back(
            transactionUUID,
            maximalClaimingBlockNumber);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

bool PaymentTransactionsHandler::isTransactionPresent(
    const TransactionUUID &transactionUUID)
{
    string query = "SELECT 1 FROM "
                   + mTableName + " WHERE uuid = ? LIMIT 1";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::isTransactionPresent: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::isTransactionPresent: "
                          "Bad binding of transactionUUID; sqlite error: " + to_string(rc));
    }

    bool result = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

void PaymentTransactionsHandler::deleteRecord(
    const TransactionUUID &transactionUUID)
{
    string query = "DELETE FROM " + mTableName + " WHERE uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::delete: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentTransactionsHandler::delete: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare deleting is completed successfully";
#endif
    } else {
        throw IOError("PaymentTransactionsHandler::delete: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

LoggerStream PaymentTransactionsHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream PaymentTransactionsHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string PaymentTransactionsHandler::logHeader() const
{
    stringstream s;
    s << "[PaymentTransactionsHandler]";
    return s.str();
}
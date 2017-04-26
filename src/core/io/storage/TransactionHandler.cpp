#include "TransactionHandler.h"

TransactionHandler::TransactionHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger *logger):

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false){

    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (transaction_uuid BLOB NOT NULL, "
                       "transaction_body BLOB NOT NULL, "
                       "transaction_bytes_count INT NOT NULL);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::creating table: Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("TransactionHandler::creating table: Run query");
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_transaction_uuid_idx on " + mTableName + " (transaction_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::creating index for TransactionUUID: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("TransactionHandler::creating index for TransactionUUID: "
                          "Run query");
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void TransactionHandler::prepareInserted() {

    if (isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call prepareInsertred, but previous transaction isn't finished";
#endif
        return;
    }

    string query = "BEGIN TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::prepareInserted: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction begin";
#endif
    } else {
        throw IOError("TransactionHandler::prepareInserted: "
                          "Run query");
    }
    isTransactionBegin = true;
}

bool TransactionHandler::commit() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call commit, but trunsaction wasn't started";
#endif
        return true;
    }

    string query = "END TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::commit: Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction commit";
#endif
        isTransactionBegin = false;
        return true;
    } else if (rc == SQLITE_BUSY) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "database busy";
#endif
        return false;
    } else {
        info() << "commit error: " << rc;
        throw IOError("TransactionHandler::commit: "
                          "Run query");
    }
}

void TransactionHandler::rollBack() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call rollBack, but trunsaction wasn't started";
#endif
        return;
    }

    string query = "ROLLBACK;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::rollback: Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("TransactionHandler::rollback: "
                          "Run query");
    }
    isTransactionBegin = false;
}

void TransactionHandler::saveRecord(
    const TransactionUUID &transactionUUID,
    BytesShared transaction,
    size_t transactionBytesCount) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "INSERT OR REPLACE INTO " + mTableName +
                   " (transaction_uuid, transaction_body, transaction_bytes_count) VALUES(?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::insert or replace: "
                          "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::insert or replace: "
                          "Bad binding of TransactionUUID");
    }
    rc = sqlite3_bind_blob(stmt, 2, transaction.get(), (int)transactionBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::insert or replace: "
                          "Bad binding of Transaction body");
    }
    rc = sqlite3_bind_int(stmt, 3, (int)transactionBytesCount);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::insert or replace: "
                          "Bad binding of Transaction bytes count");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting or replacing is completed successfully";
#endif
    } else {
        throw IOError("TransactionHandler::insert or replace: "
                          "Run query");
    }
}

void TransactionHandler::deleteRecord(
    const TransactionUUID &transactionUUID) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "DELETE FROM " + mTableName + " WHERE transaction_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::delete: "
                          "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::delete: "
                          "Bad binding of TransactionUUID");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare deleting is completed successfully";
#endif
    } else {
        throw IOError("PaymentOperationStateHandler::delete: "
                          "Run query");
    }
}

pair<BytesShared, size_t> TransactionHandler::getTransaction(
    const TransactionUUID &transactionUUID) {

    string query = "SELECT transaction_body, transaction_bytes_count FROM " + mTableName + " WHERE transaction_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::getTransaction: "
                          "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TransactionHandler::getTransaction: "
                          "Bad binding of TransactionUUID");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        size_t stateBytesCount = (size_t)sqlite3_column_int(stmt, 1);
        BytesShared state = tryMalloc(stateBytesCount);
        memcpy(
            state.get(),
            sqlite3_column_blob(stmt, 0),
            stateBytesCount);
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return make_pair(state, stateBytesCount);
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("TransactionHandler::getTransaction: "
                                "There are now records with requested transactionUUID");
    }
}

void TransactionHandler::closeConnection() {

    if (mDataBase != nullptr) {
        sqlite3_close_v2(mDataBase);
    }
}

LoggerStream TransactionHandler::info() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream TransactionHandler::error() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string TransactionHandler::logHeader() const {

    stringstream s;
    s << "[TransactionHandler]";
    return s.str();
}

#include "PaymentOperationStateHandler.h"

PaymentOperationStateHandler::PaymentOperationStateHandler(
    const string &dataBasePath,
    const string &tableName,
    Logger *logger):

    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false){

    int rc = sqlite3_open_v2(dataBasePath.c_str(), &mDataBase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc == SQLITE_OK) {
    } else {
        throw IOError("PaymentOperationStateHandler::connection "
                          "Can't open database " + dataBasePath);
    }

    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
        " (transaction_uuid BLOB NOT NULL, "
            "state BLOB NOT NULL, "
            "state_bytes_count INT NOT NULL);";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::creating table: " + mTableName +
                      " : Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PaymentOperationStateHandler::creating table: " + mTableName +
                      " : Run query");
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_transaction_uuid_idx on " + mTableName + " (transaction_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::creating index for TransactionUUID: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PaymentOperationStateHandler::creating index for TransactionUUID: "
                          "Run query");
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void PaymentOperationStateHandler::prepareInserted() {

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
        throw IOError("PaymentOperationStateHandler::prepareInserted: "
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
        throw IOError("PaymentOperationStateHandler::prepareInserted: "
                          "Run query");
    }
    isTransactionBegin = true;
}

bool PaymentOperationStateHandler::commit() {

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
        throw IOError("PaymentOperationStateHandler::commit: "
                          "Bad query");
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
        throw IOError("PaymentOperationStateHandler::commit: "
                          "Run query");
    }
}

void PaymentOperationStateHandler::rollBack() {

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
        throw IOError("PaymentOperationStateHandler::rollback: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("PaymentOperationStateHandler::rollback: "
                          "Run query");
    }

    isTransactionBegin = false;
}

void PaymentOperationStateHandler::insert(
    const TransactionUUID &transactionUUID,
    BytesShared state,
    size_t stateBytesCount) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "INSERT INTO " + mTableName +
        " (transaction_uuid, state, state_bytes_count) VALUES(?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert: "
                          "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert: "
                          "Bad binding of TransactionUUID");
    }
    rc = sqlite3_bind_blob(stmt, 2, state.get(), (int)stateBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert: "
                          "Bad binding of State");
    }
    rc = sqlite3_bind_int(stmt, 3, (int)stateBytesCount);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert: "
                          "Bad binding of State bytes count");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("PaymentOperationStateHandler::insert: "
                          "Run query");
    }
}

void PaymentOperationStateHandler::update(
    const TransactionUUID &transactionUUID,
    BytesShared state,
    size_t stateBytesCount) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "UPDATE " + mTableName + " SET state = ?, state_bytes_count = ? WHERE transaction_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::update: "
                          "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, state.get(), (int)stateBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::update: "
                          "Bad binding of State");
    }
    rc = sqlite3_bind_int(stmt, 2, (int)stateBytesCount);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert: "
                          "Bad binding of State bytes count");
    }
    rc = sqlite3_bind_blob(stmt, 3, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::update: "
                          "Bad binding of TransactionUUID");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare updating is completed successfully";
#endif
    } else {
        throw IOError("PaymentOperationStateHandler::update: "
                          "Run query");
    }
}

void PaymentOperationStateHandler::saveRecord(
    const TransactionUUID &transactionUUID,
    BytesShared state,
    size_t stateBytesCount) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "INSERT OR REPLACE INTO " + mTableName +
        " (transaction_uuid, state, state_bytes_count) VALUES(?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert or replace: "
                          "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert or replace: "
                          "Bad binding of TransactionUUID");
    }
    rc = sqlite3_bind_blob(stmt, 2, state.get(), (int)stateBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert or replace: "
                          "Bad binding of State");
    }
    rc = sqlite3_bind_int(stmt, 3, (int)stateBytesCount);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::insert or replace: "
                          "Bad binding of State bytes count");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting or replacing is completed successfully";
#endif
    } else {
        throw IOError("PaymentOperationStateHandler::insert or replace: "
                          "Run query");
    }
}

void PaymentOperationStateHandler::deleteRecord(
    const TransactionUUID &transactionUUID) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "DELETE FROM " + mTableName + " WHERE transaction_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::delete: "
                          "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::delete: "
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

pair<BytesShared, size_t> PaymentOperationStateHandler::getState(
    const TransactionUUID &transactionUUID) {

    string query = "SELECT state, state_bytes_count FROM " + mTableName + " WHERE transaction_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::getState: "
                          "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::delete: "
                          "Bad binding of TransactionUUID");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_ROW) {
        size_t stateBytesCount = (size_t)sqlite3_column_int(stmt, 1);
        BytesShared state = tryMalloc(stateBytesCount);
        memcpy(
            state.get(),
            sqlite3_column_blob(stmt, 0),
            stateBytesCount);
        return make_pair(state, stateBytesCount);
    } else {
        throw NotFoundError("PaymentOperationStateHandler::getState: "
                                "There are now records with requested transactionUUID");
    }
}

void PaymentOperationStateHandler::closeConnection() {

    sqlite3_close_v2(mDataBase);
}

LoggerStream PaymentOperationStateHandler::info() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream PaymentOperationStateHandler::error() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string PaymentOperationStateHandler::logHeader() const {

    stringstream s;
    s << "[PaymentOperationStateHandler]";
    return s.str();
}

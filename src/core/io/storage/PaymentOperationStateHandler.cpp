#include "PaymentOperationStateHandler.h"

PaymentOperationStateHandler::PaymentOperationStateHandler(
    sqlite3 *db,
    const string &tableName,
    Logger *logger):

    mDataBase(db),
    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false){

    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
        " (transaction_uuid BLOB NOT NULL, "
            "state BLOB NOT NULL, "
            "state_bytes_count INT NOT NULL);";
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << query;
#endif
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::creating table: " + mTableName +
                      " : Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "table" << mTableName << "created successfully";
#endif
    } else {
        throw IOError("PaymentOperationStateHandler::creating table: " + mTableName +
                      " : Run query");
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_transaction_uuid_idx on " + mTableName + " (transaction_uuid);";
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << query;
#endif
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::creating index for TransactionUUID: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "index for transaction created successfully";
#endif
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

    sqlite3_finalize(stmt);
    string query = "BEGIN TRANSACTION;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::prepareInserted: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
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

void PaymentOperationStateHandler::commit() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call commit, but trunsaction wasn't started";
#endif
        return;
    }

    string query = "END TRANSACTION;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::commit: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction commit";
#endif
    } else {
        throw IOError("PaymentOperationStateHandler::commit: "
                          "Run query");
    }

    sqlite3_reset(stmt);
    isTransactionBegin = false;
}

void PaymentOperationStateHandler::rollBack() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call rollBack, but trunsaction wasn't started";
#endif
        return;
    }

    sqlite3_finalize(stmt);
    string query = "ROLLBACK;";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::rollback: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("PaymentOperationStateHandler::rollback: "
                          "Run query");
    }

    sqlite3_reset(stmt);
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
    if (sqlite3_step(stmt) == SQLITE_ROW) {
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

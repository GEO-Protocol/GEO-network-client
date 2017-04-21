#include "TrustLineHandler.h"

TrustLineHandler::TrustLineHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger *logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false) {

    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(contractor BLOB NOT NULL, "
                   "incoming_amount BLOB NOT NULL, "
                   "outgoing_amount BLOB NOT NULL, "
                   "balance BLOB NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::creating table: Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("TrustLineHandler::creating table: Run query");
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_contractor_idx on " + mTableName + "(contractor);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::creating index for Contractor: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("TrustLineHandler::creating index for Contractor: "
                              "Run query");
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

bool TrustLineHandler::commit() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call commit, but trunsaction wasn't started";
#endif
        return true;
    }

    string query = "COMMIT TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        info() << "commit query error: " << rc;
        throw IOError("TrustLineHandler::commit: Bad query");
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
        throw IOError("TrustLineHandler::commit: Run query");
    }
}

void TrustLineHandler::rollBack() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call rollBack, but trunsaction wasn't started";
#endif
        return;
    }

    string query = "ROLLBACK TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::rollback: Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("TrustLineHandler::rollback: Run query");
    }

    isTransactionBegin = false;
}

void TrustLineHandler::prepareInserted() {

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
        throw IOError("TrustLineHandler::prepareInserted: Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction begin";
#endif
    } else {
        throw IOError("TrustLineHandler::prepareInserted: Run query");
    }
    isTransactionBegin = true;
}

vector<TrustLine::Shared> TrustLineHandler::allTrustLines () {

    string queryCount = "SELECT count(*) FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, queryCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::trustLines: Bad count query");
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<TrustLine::Shared> result;
    result.reserve(rowCount);

    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " + mTableName;
    rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::trustLines: Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID contractor((uint8_t*)sqlite3_column_blob(stmt, 0));

        byte* incomingAmountBytes = (byte*)sqlite3_column_blob(stmt, 1);
        vector<byte> incomingAmountBufferBytes(
            incomingAmountBytes,
            incomingAmountBytes + kTrustLineAmountBytesCount);
        TrustLineAmount incomingAmount = bytesToTrustLineAmount(incomingAmountBufferBytes);

        byte* outgoingAmountBytes = (byte*)sqlite3_column_blob(stmt, 2);
        vector<byte> outgoingAmountBufferBytes(
            outgoingAmountBytes,
            outgoingAmountBytes + kTrustLineAmountBytesCount);
        TrustLineAmount outgoingAmount = bytesToTrustLineAmount(outgoingAmountBufferBytes);

        byte* balanceBytes = (byte*)sqlite3_column_blob(stmt, 3);
        vector<byte> balanceBufferBytes(
                balanceBytes,
                balanceBytes + kTrustLineBalanceSerializeBytesCount);
        TrustLineBalance balance = bytesToTrustLineBalance(balanceBufferBytes);

        try {
            result.push_back(
                make_shared<TrustLine>(
                    contractor,
                    incomingAmount,
                    outgoingAmount,
                    balance));
        } catch (...) {
            throw Exception("TrustLinesManager::loadTrustLine. "
                                "Unable to create trust line instance from DB.");
        }

    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

void TrustLineHandler::deleteTrustLine(
    const NodeUUID &contractorUUID) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "DELETE FROM " + mTableName + " WHERE contractor = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::deleteTrustLine: Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::deleteTrustLine: "
                              "Bad binding of Contractor");
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::deleteTrustLine: Run query");
    }
}

bool TrustLineHandler::containsContractor(
    const NodeUUID &contractorUUID) {

    string query = "SELECT contractor FROM " + mTableName + " WHERE contractor = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::containsContractor: Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::containsContractor: "
                              "Bad binding of Contractor");
    }
    return sqlite3_step(stmt) == SQLITE_ROW;
}

void TrustLineHandler::saveTrustLine(
    TrustLine::Shared trustLine) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "INSERT OR REPLACE INTO " + mTableName +
                   "(contractor, incoming_amount, outgoing_amount, balance) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert or replace: "
                          "Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, trustLine->contractorNodeUUID().data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert or replace: "
                          "Bad binding of Contractor");
    }

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(trustLine->incomingTrustAmount());
    rc = sqlite3_bind_blob(stmt, 2, incomingAmountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert or replace: "
                          "Bad binding of Incoming Amount");
    }

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(trustLine->outgoingTrustAmount());
    rc = sqlite3_bind_blob(stmt, 3, outgoingAmountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert or replace: "
                          "Bad binding of Outgoing Amount");
    }

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(const_cast<TrustLineBalance&>(trustLine->balance()));
    rc = sqlite3_bind_blob(stmt, 4, balanceBufferBytes.data(), kTrustLineBalanceSerializeBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert or replace: "
                          "Bad binding of Balance");
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting or replacing is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::insert or replace: Run query");
    }
}

LoggerStream TrustLineHandler::info() const {
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream TrustLineHandler::error() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string TrustLineHandler::logHeader() const {
    stringstream s;
    s << "[TrustLineHandler]";
    return s.str();
}



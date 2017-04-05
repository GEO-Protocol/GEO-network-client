#include "TrustLineHandler.h"

TrustLineHandler::TrustLineHandler(
    sqlite3 *db,
    string tableName,
    Logger *logger) :

    mDataBase(db),
    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false) {

    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(contractor BLOB NOT NULL, "
                   "incoming_amount BLOB NOT NULL, "
                   "outgoing_amount BLOB NOT NULL, "
                   "balance BLOB NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::creating table: " + mTableName +
                      " : Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "table" << mTableName << "created successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::creating table: " + mTableName +
                      " : Run query");
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
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "index for contractor created successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::creating index for Contractor: "
                              "Run query");
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void TrustLineHandler::insert(
    TrustLine::Shared trustLine) {

    if (!isTransactionBegin) {
        prepareInsertred();
    }

    string query = "INSERT INTO " + mTableName +
                   "(contractor, incoming_amount, outgoing_amount, balance) VALUES (?, ?, ?, ?);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert: "
                              "Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, trustLine->contractorNodeUUID().data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert: "
                              "Bad binding of Contractor");
    }

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(trustLine->incomingTrustAmount());
    rc = sqlite3_bind_blob(stmt, 2, incomingAmountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert: "
                              "Bad binding of Incoming Amount");
    }

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(trustLine->outgoingTrustAmount());
    rc = sqlite3_bind_blob(stmt, 3, outgoingAmountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert: "
                              "Bad binding of Outgoing Amount");
    }

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(const_cast<TrustLineBalance&>(trustLine->balance()));
    rc = sqlite3_bind_blob(stmt, 4, balanceBufferBytes.data(), kTrustLineBalanceSerializeBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert: "
                              "Bad binding of Balance");
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting (" << trustLine->contractorNodeUUID() <<
        ", " << trustLine->incomingTrustAmount() << ", " << trustLine->outgoingTrustAmount() <<
        ", " << trustLine->balance() << ") " << "is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::insert: Run query");
    }
}

void TrustLineHandler::commit() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call commit, but trunsaction wasn't started";
#endif
        return;
    }

    string query = "END TRANSACTION;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::commit: Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction commit";
#endif
    } else {
        throw IOError("TrustLineHandler::commit: Run query");
    }

    sqlite3_reset(stmt);
    isTransactionBegin = false;
}

void TrustLineHandler::rollBack() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call rollBack, but trunsaction wasn't started";
#endif
        return;
    }

    sqlite3_finalize(stmt);
    string query = "ROLLBACK;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::rollback: Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("TrustLineHandler::rollback: Run query");
    }

    sqlite3_reset(stmt);
    isTransactionBegin = false;
}

void TrustLineHandler::prepareInsertred() {

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
        throw IOError("TrustLineHandler::prepareInsertred: Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction begin";
#endif
    } else {
        throw IOError("TrustLineHandler::prepareInsertred: Run query");
    }
    isTransactionBegin = true;
}

vector<TrustLine::Shared> TrustLineHandler::trustLines() {

    string queryCount = "SELECT count(*) FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, queryCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::trustLines: Bad count query");
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    info() << "trustLines\t count records: " << rowCount;
    vector<TrustLine::Shared> result;
    result.reserve(rowCount);

    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " + mTableName;
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "select: " << query;
#endif
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

        info() << "read one trust line: " <<
                      contractor << " " <<
                      incomingAmount << " " <<
                      outgoingAmount << " " <<
                      balance;
        result.push_back(
            make_shared<TrustLine>(
                contractor,
                incomingAmount,
                outgoingAmount,
                balance));
    }
    sqlite3_reset(stmt);
    return result;
}

void TrustLineHandler::deleteTrustLine(const NodeUUID &contractorUUID) {

    if (!isTransactionBegin) {
        prepareInsertred();
    }

    string query = "DELETE FROM " + mTableName + " WHERE contractor = ?";
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "delete: " << query;
#endif
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
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::deleteTrustLine: Run query");
    }
}

void TrustLineHandler::update(TrustLine::Shared trustLine) {

    if (!isTransactionBegin) {
        prepareInsertred();
    }

    string query = "UPDATE " + mTableName +
        " SET incoming_amount = ?, outgoing_amount = ?, balance = ? " +
        "WHERE contractor = ?";
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "update: " << query;
#endif
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::update: Bad query");
    }

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(trustLine->incomingTrustAmount());
    rc = sqlite3_bind_blob(stmt, 1, incomingAmountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::update: "
                              "Bad binding of Incoming Amount");
    }

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(trustLine->outgoingTrustAmount());
    rc = sqlite3_bind_blob(stmt, 2, outgoingAmountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::update: "
                              "Bad binding of Outgoing Amount");
    }

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(const_cast<TrustLineBalance&>(trustLine->balance()));
    rc = sqlite3_bind_blob(stmt, 3, balanceBufferBytes.data(), kTrustLineBalanceSerializeBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::update: "
                              "Bad binding of Balance");
    }

    rc = sqlite3_bind_blob(stmt, 4, trustLine->contractorNodeUUID().data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::update: "
                              "Bad binding of Contractor");
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "updating is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::update: Run query");
    }
}

bool TrustLineHandler::containsContractor(const NodeUUID &contractorUUID) {

    string query = "SELECT contractor FROM " + mTableName + " WHERE contractor = ?";
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "contains: " << query;
#endif

    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::containsContractor: Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::update: "
                              "Bad binding of Contractor");
    }

    return sqlite3_step(stmt) == SQLITE_ROW;
}

void TrustLineHandler::saveTrustLine(TrustLine::Shared trustLine) {

    if (containsContractor(trustLine->contractorNodeUUID())) {
        update(trustLine);
    } else {
        insert(trustLine);
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



#include "AuditHandler.h"

AuditHandler::AuditHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(id INTEGER PRIMARY KEY, "
                   "trust_line_id INTEGER NOT NULL, "
                   "our_key_hash INTEGER NOT NULL, "
                   "our_sign BLOB NOT NULL, "
                   "our_sign_bytes_count INT NOT NULL, "
                   "contractor_key_hash INTEGER NOT NULL, "
                   "contractor_sign BLOB NOT NULL, "
                   "contractor_sign_bytes_count INT NOT NULL, "
                   "balance BLOB NOT NULL, "
                   "outgoing_amount BLOB NOT NULL, "
                   "incoming_amount BLOB NOT NULL, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("AuditHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_id_trust_line_id_idx on " + mTableName + "(id, trust_line_id);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::creating  index for ID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("AuditHandler::creating index for ID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void AuditHandler::saveAudit(
    uint32_t id,
    uint32_t TrustLineID,
    uint32_t ownKeyHash,
    BytesShared ownSign,
    size_t ownSignBytesCount,
    uint32_t contractorKeyHash,
    BytesShared contractorSign,
    size_t contractorSignBytesCount,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount,
    const TrustLineBalance &balance)
{
    string query = "INSERT INTO " + mTableName +
                   "(id, trust_line_id, our_key_hash, our_sign, our_sign_bytes_count, contractor_key_hash, "
                   "contractor_sign, contractor_sign_bytes_count, incoming_amount, outgoing_amount, "
                   "balance) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, TrustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, ownKeyHash);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of OwnKeyHash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 4, ownSign.get(), (int)ownSignBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of OnwSign; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 5, (int)ownSignBytesCount);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of OnwSign bytes count; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 6, contractorKeyHash);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of ContractorKeyHash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 7, contractorSign.get(), (int)contractorSignBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of ContractorSign; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 8, (int)contractorSignBytesCount);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of ContractorSign bytes count; sqlite error: " + to_string(rc));
    }
    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(incomingAmount);
    rc = sqlite3_bind_blob(stmt, 9, incomingAmountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of Incoming Amount; sqlite error: " + to_string(rc));
    }
    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(outgoingAmount);
    rc = sqlite3_bind_blob(stmt, 10, outgoingAmountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of Outgoing Amount; sqlite error: " + to_string(rc));
    }
    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(const_cast<TrustLineBalance&>(balance));
    rc = sqlite3_bind_blob(stmt, 11, balanceBufferBytes.data(), kTrustLineBalanceSerializeBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveAudit: "
                          "Bad binding of Balance; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("AuditHandler::saveAudit: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

LoggerStream AuditHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream AuditHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string AuditHandler::logHeader() const
{
    stringstream s;
    s << "[AuditHandler]";
    return s.str();
}
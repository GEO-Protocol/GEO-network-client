#include "OutgoingPaymentReceiptHandler.h"

OutgoingPaymentReceiptHandler::OutgoingPaymentReceiptHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (trust_line_id INTEGER NOT NULL, "
                   "audit_number INTEGER NOT NULL, "
                   "transaction_uuid BLOB NOT NULL, "
                   "own_public_key_hash BLOB NOT NULL, "
                   "amount BLOB NOT NULL, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id) ON DELETE CASCADE ON UPDATE CASCADE, "
                   "FOREIGN KEY(own_public_key_hash) REFERENCES own_keys(hash) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("OutgoingPaymentReceiptHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_trust_line_id_audit_number_key_hash_idx on " + mTableName + "(trust_line_id, audit_number, own_public_key_hash);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::creating  index for TrustLineID and AuditNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("OutgoingPaymentReceiptHandler::creating index for TrustLineID and AuditNumber: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_transaction_uuid_idx on " + mTableName + "(transaction_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::creating  index for TransactionUUID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("OutgoingPaymentReceiptHandler::creating index for TransactionUUID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void OutgoingPaymentReceiptHandler::saveRecord(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber,
    const TransactionUUID &transactionUUID,
    const KeyHash::Shared ownPublicKeyHash,
    const TrustLineAmount &amount)
{
    string query = "INSERT INTO " + mTableName +
                   "(trust_line_id, audit_number, transaction_uuid, own_public_key_hash, "
                   "amount) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::saveRecord: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of AuditNumber; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, transactionUUID.data,
                           (int)TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 4, ownPublicKeyHash->data(),
                           (int)KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of ContractorPublicKeyHash; sqlite error: " + to_string(rc));
    }
    vector<byte> amountBufferBytes = trustLineAmountToBytes(amount);
    rc = sqlite3_bind_blob(stmt, 5, amountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of Amount; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("OutgoingPaymentReceiptHandler::saveRecord: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

vector<TrustLineAmount> OutgoingPaymentReceiptHandler::auditAmounts(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber)
{
    vector<TrustLineAmount> result;
    sqlite3_stmt *stmt;
    string query = "SELECT amount FROM " + mTableName + " WHERE trust_line_id = ? AND audit_number = ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::auditAmounts: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::auditAmounts: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::auditAmounts: "
                          "Bad binding of AuditNumber; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {

        auto amountBytes = (byte*)sqlite3_column_blob(stmt, 0);
        vector<byte> incomingAmountBufferBytes(
            amountBytes,
            amountBytes + kTrustLineAmountBytesCount);

        result.push_back(
            bytesToTrustLineAmount(
                incomingAmountBufferBytes));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<ReceiptRecord::Shared> OutgoingPaymentReceiptHandler::receiptsByAuditNumber(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber)
{
    vector<ReceiptRecord::Shared> result;
    sqlite3_stmt *stmt;
    string query = "SELECT amount, transaction_uuid, own_public_key_hash FROM " + mTableName
                   + " WHERE trust_line_id = ? AND audit_number = ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::receiptsByAuditNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::receiptsByAuditNumber: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::receiptsByAuditNumber: "
                          "Bad binding of AuditNumber; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {

        auto amountBytes = (byte*)sqlite3_column_blob(stmt, 0);
        vector<byte> incomingAmountBufferBytes(
            amountBytes,
            amountBytes + kTrustLineAmountBytesCount);

        TransactionUUID transactionUUID((uint8_t*)sqlite3_column_blob(stmt, 1));

        auto ownKeyHash = make_shared<lamport::KeyHash>(
            (byte*)sqlite3_column_blob(stmt, 2));

        result.push_back(make_shared<ReceiptRecord>(
            auditNumber,
            transactionUUID,
            bytesToTrustLineAmount(
                incomingAmountBufferBytes),
            ownKeyHash,
            nullptr));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream OutgoingPaymentReceiptHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream OutgoingPaymentReceiptHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string OutgoingPaymentReceiptHandler::logHeader() const
{
    stringstream s;
    s << "[OutgoingPaymentReceiptHandler]";
    return s.str();
}
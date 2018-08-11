#include "IncomingPaymentReceiptHandler.h"

IncomingPaymentReceiptHandler::IncomingPaymentReceiptHandler(
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
                   "contractor_public_key_hash BLOB NOT NULL, "
                   "amount BLOB NOT NULL, "
                   "contractor_signature BLOB NOT NULL, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id) ON DELETE CASCADE ON UPDATE CASCADE, "
                   "FOREIGN KEY(contractor_public_key_hash) REFERENCES contractor_keys(hash) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("IncomingPaymentReceiptHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_trust_line_id_audit_number_key_hash_idx on " + mTableName + "(trust_line_id, audit_number, contractor_public_key_hash);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::creating  index for TrustLineID and AuditNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("IncomingPaymentReceiptHandler::creating index for TrustLineID and AuditNumber: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_transaction_uuid_idx on " + mTableName + "(transaction_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::creating  index for TransactionUUID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("IncomingPaymentReceiptHandler::creating index for TransactionUUID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void IncomingPaymentReceiptHandler::saveRecord(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber,
    const TransactionUUID &transactionUUID,
    const KeyHash::Shared contractorPublicKeyHash,
    const TrustLineAmount &amount,
    const Signature::Shared contractorSignature)
{
    string query = "INSERT INTO " + mTableName +
                   "(trust_line_id, audit_number, transaction_uuid, contractor_public_key_hash, "
                   "amount, contractor_signature) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::saveRecord: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditNumber);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of AuditNumber; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, transactionUUID.data,
                           (int)TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 4, contractorPublicKeyHash->data(),
                           (int)KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of ContractorPublicKeyHash; sqlite error: " + to_string(rc));
    }
    vector<byte> amountBufferBytes = trustLineAmountToBytes(amount);
    rc = sqlite3_bind_blob(stmt, 5, amountBufferBytes.data(), kTrustLineAmountBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of Amount; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 6, contractorSignature->data(),
                           (int)contractorSignature->signatureSize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::saveRecord: "
                          "Bad binding of ContractorSignature; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("IncomingPaymentReceiptHandler::saveRecord: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

vector<TrustLineAmount> IncomingPaymentReceiptHandler::auditAmounts(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber)
{
    vector<TrustLineAmount> result;
    sqlite3_stmt *stmt;
    string query = "SELECT amount FROM " + mTableName + " WHERE trust_line_id = ? AND audit_number = ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::auditAmounts: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::auditAmounts: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditNumber);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::auditAmounts: "
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

vector<ReceiptRecord::Shared> IncomingPaymentReceiptHandler::receiptsByAuditNumber(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber)
{
    vector<ReceiptRecord::Shared> result;
    sqlite3_stmt *stmt;
    string query = "SELECT amount, transaction_uuid, contractor_public_key_hash, contractor_signature FROM "
                   + mTableName + " WHERE trust_line_id = ? AND audit_number = ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::receiptsByAuditNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::receiptsByAuditNumber: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditNumber);
    if (rc != SQLITE_OK) {
        throw IOError("IncomingPaymentReceiptHandler::receiptsByAuditNumber: "
                          "Bad binding of AuditNumber; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {

        auto amountBytes = (byte*)sqlite3_column_blob(stmt, 0);
        vector<byte> incomingAmountBufferBytes(
            amountBytes,
            amountBytes + kTrustLineAmountBytesCount);

        TransactionUUID transactionUUID((uint8_t*)sqlite3_column_blob(stmt, 1));

        auto contractorKeyHash = make_shared<lamport::KeyHash>(
            (byte*)sqlite3_column_blob(stmt, 2));

        auto contractorSignature = make_shared<lamport::Signature>(
            (byte*)sqlite3_column_blob(stmt, 3));

        result.push_back(make_shared<ReceiptRecord>(
            auditNumber,
            transactionUUID,
            bytesToTrustLineAmount(
                incomingAmountBufferBytes),
            contractorKeyHash,
            contractorSignature));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream IncomingPaymentReceiptHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream IncomingPaymentReceiptHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string IncomingPaymentReceiptHandler::logHeader() const
{
    stringstream s;
    s << "[IncomingPaymentReceiptHandler]";
    return s.str();
}
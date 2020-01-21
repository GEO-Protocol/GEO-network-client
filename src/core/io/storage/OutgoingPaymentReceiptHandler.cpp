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
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
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
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
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
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
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
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
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

vector<pair<TransactionUUID, TrustLineAmount>> OutgoingPaymentReceiptHandler::auditAmounts(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber)
{
    vector<pair<TransactionUUID, TrustLineAmount>> result;
    sqlite3_stmt *stmt;
    string query = "SELECT transaction_uuid, amount FROM " + mTableName + " WHERE trust_line_id = ? AND audit_number = ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
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

        TransactionUUID transactionUUID(
            (byte*)sqlite3_column_blob(stmt, 0));

        auto amountBytes = (byte*)sqlite3_column_blob(stmt, 1);
        vector<byte> amountBufferBytes(
            amountBytes,
            amountBytes + kTrustLineAmountBytesCount);
        auto amount = bytesToTrustLineAmount(
            amountBufferBytes);

        result.emplace_back(
            transactionUUID,
            amount);
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
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
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

vector<ReceiptRecord::Shared> OutgoingPaymentReceiptHandler::receiptsLessEqualThanAuditNumber(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber)
{
    vector<ReceiptRecord::Shared> result;
    sqlite3_stmt *stmt;
    string query = "SELECT amount, transaction_uuid, own_public_key_hash FROM " + mTableName
                   + " WHERE trust_line_id = ? AND audit_number <= ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::receiptsLessEqualThanAuditNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::receiptsLessEqualThanAuditNumber: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::receiptsLessEqualThanAuditNumber: "
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

uint32_t OutgoingPaymentReceiptHandler::countReceiptsByNumber(
    const TrustLineID trustLineID,
    const AuditNumber auditNumber)
{
    sqlite3_stmt *stmt;
    string query = "SELECT COUNT(transaction_uuid) FROM "
                   + mTableName + " WHERE trust_line_id = ? AND audit_number = ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::countReceiptsByNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::countReceiptsByNumber: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::countReceiptsByNumber: "
                          "Bad binding of AuditNumber; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto countReceipts = (uint32_t)sqlite3_column_int(stmt, 0);

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return countReceipts;
}

void OutgoingPaymentReceiptHandler::deleteRecords(
    const TransactionUUID &transactionUUID)
{
    string query = "DELETE FROM " + mTableName + " WHERE transaction_uuid = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::deleteRecords: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::deleteRecords: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("OutgoingPaymentReceiptHandler::deleteRecords: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void OutgoingPaymentReceiptHandler::deleteRecords(
    const TrustLineID trustLineID)
{
    string query = "DELETE FROM " + mTableName + " WHERE trust_line_id = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::deleteRecordsByTrustLineID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::deleteRecordsByTrustLineID: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("OutgoingPaymentReceiptHandler::deleteRecordsByTrustLineID: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

bool OutgoingPaymentReceiptHandler::isContainsKeyHash(
    KeyHash::Shared keyHash) const
{
    sqlite3_stmt *stmt;
    string query = "SELECT own_public_key_hash FROM "
                   + mTableName + " WHERE own_public_key_hash = ? LIMIT 1";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::isContainsKeyHash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, keyHash->data(),
                           (int)KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::isContainsKeyHash: "
                          "Bad binding of OwnPublicKeyHash; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto result = (rc == SQLITE_ROW);

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

bool OutgoingPaymentReceiptHandler::isContainsTransaction(
    const TransactionUUID &transactionUUID) const
{
    sqlite3_stmt *stmt;
    string query = "SELECT transaction_uuid FROM "
                   + mTableName + " WHERE transaction_uuid = ? LIMIT 1";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::isContainsTransaction: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OutgoingPaymentReceiptHandler::isContainsTransaction: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto result = (rc == SQLITE_ROW);

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
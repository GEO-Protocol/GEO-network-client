#include "PaymentParticipantsVotesHandler.h"

PaymentParticipantsVotesHandler::PaymentParticipantsVotesHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger):

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (transaction_uuid BLOB NOT NULL, "
                   "node_uuid BLOB NOT NULL, "
                   "payment_node_id INTEGER NOT NULL, "
                   "public_key BLOB NOT NULL, "
                   "signature BLOB NOT NULL, "
                   "recording_time INTEGER NOT NULL);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PaymentParticipantsVotesHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_transaction_uuid_idx on " + mTableName + " (transaction_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::creating index for TransactionUUID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PaymentParticipantsVotesHandler::creating index for TransactionUUID: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void PaymentParticipantsVotesHandler::saveRecord(
    const TransactionUUID &transactionUUID,
    const NodeUUID &nodeUUID,
    const PaymentNodeID paymentNodeID,
    const lamport::PublicKey::Shared publicKey,
    const lamport::Signature::Shared signature)
{
    string query = "INSERT INTO " + mTableName + " (transaction_uuid, node_uuid, "
                        "payment_node_id, public_key, signature, recording_time) VALUES(?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::saveRecord: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::saveRecord: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, nodeUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::saveRecord: "
                          "Bad binding of NodeUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, paymentNodeID);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::saveRecord: "
                          "Bad binding of PaymentNodeID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 4, publicKey->data(), (int)publicKey->keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::saveRecord: "
                              "Bad binding of Public Key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 5, signature->data(), (int)signature->signatureSize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::saveRecord: "
                              "Bad binding of Signature; sqlite error: " + to_string(rc));
    }
    GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(utc_now());
    rc = sqlite3_bind_int64(stmt, 6, timestamp);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::saveRecord: "
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
        throw IOError("PaymentParticipantsVotesHandler::saveRecord: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

map<PaymentNodeID, lamport::Signature::Shared> PaymentParticipantsVotesHandler::participantsSignatures(
    const TransactionUUID &transactionUUID)
{
    string query = "SELECT payment_node_id, signature FROM " + mTableName + " WHERE transaction_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentOperationStateHandler::byTransaction: "
                              "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentParticipantsVotesHandler::participantsSignatures: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    map<PaymentNodeID, lamport::Signature::Shared> result;
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        auto paymentNodeID = (PaymentNodeID)sqlite3_column_int(stmt, 0);
        auto signature = make_shared<lamport::Signature>(
            (byte*)sqlite3_column_blob(stmt, 1));
        result.insert(
            make_pair(
                paymentNodeID,
                signature));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream PaymentParticipantsVotesHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream PaymentParticipantsVotesHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string PaymentParticipantsVotesHandler::logHeader() const
{
    stringstream s;
    s << "[PaymentParticipantsVotesHandler]";
    return s.str();
}
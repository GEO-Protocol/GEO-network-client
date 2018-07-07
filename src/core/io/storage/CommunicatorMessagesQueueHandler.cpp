#include "CommunicatorMessagesQueueHandler.h"

CommunicatorMessagesQueueHandler::CommunicatorMessagesQueueHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger):

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (contractor_uuid BLOB NOT NULL, "
                   "equivalent INT NOT NULL, "
                   "transaction_uuid BLOB NOT NULL, "
                   "message_type INT NOT NULL, "
                   "message BLOB NOT NULL, "
                   "message_bytes_count INT NOT NULL, "
                   "recording_time INT NOT NULL);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("CommunicatorMessagesQueueHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void CommunicatorMessagesQueueHandler::saveRecord(
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    const TransactionUUID &transactionUUID,
    const Message::SerializedType messageType,
    BytesShared message,
    size_t messageBytesCount)
{
    string query = "INSERT INTO " + mTableName +
                   " (contractor_uuid, equivalent, transaction_uuid, message_type, "
                   "message, message_bytes_count, recording_time) "
                   "VALUES(?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Bad binding of ContractorUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, equivalent);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Bad binding of Equivalent; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 4, (int)messageType);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Bad binding of MessageType bytes count; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 5, message.get(), (int)messageBytesCount, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Bad binding of Message; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 6, (int)messageBytesCount);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Bad binding of Message bytes count; sqlite error: " + to_string(rc));
    }
    GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(utc_now());
    rc = sqlite3_bind_int64(stmt, 7, timestamp);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Bad binding of Timestamp; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "inserting is completed successfully";
#endif
    } else if (rc == SQLITE_CONSTRAINT) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "inserting is not completed due to constraint violation";
#endif
    } else {
        throw IOError("CommunicatorMessagesQueueHandler::insert: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void CommunicatorMessagesQueueHandler::deleteRecord(
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    const Message::SerializedType messageType)
{
    string query = "DELETE FROM " + mTableName
                   + " WHERE contractor_uuid = ? AND equivalent = ? AND message_type = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Bad binding of ContractorUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, equivalent);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Bad binding of Equivalent; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, (int)messageType);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Bad binding of MessageType; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare deleting is completed successfully";
#endif
    } else {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void CommunicatorMessagesQueueHandler::deleteRecord(
    const NodeUUID &contractorUUID,
    const TransactionUUID &transactionUUID)
{
    string query = "DELETE FROM " + mTableName + " WHERE contractor_uuid = ? AND transaction_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Bad binding of ContractorUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, transactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Bad binding of TransactionUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare deleting is completed successfully";
#endif
    } else {
        throw IOError("CommunicatorMessagesQueueHandler::delete: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

vector<tuple<const NodeUUID, BytesShared, Message::SerializedType>> CommunicatorMessagesQueueHandler::allMessages()
{
    string queryCount = "SELECT count(*) FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, queryCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::allMessages: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<tuple<const NodeUUID, BytesShared, Message::SerializedType>> result;
    result.reserve(rowCount);
    string query = "SELECT contractor_uuid, message_type, message, message_bytes_count FROM "
                   + mTableName + ";";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorMessagesQueueHandler::allMessages: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {

        NodeUUID contractorUUID((uint8_t *)sqlite3_column_blob(stmt, 0));
        auto messageType = (Message::SerializedType)sqlite3_column_int(stmt, 1);

        auto messageBytesCount = (size_t) sqlite3_column_int(stmt, 3);
        BytesShared message = tryMalloc(messageBytesCount);
        memcpy(
            message.get(),
            sqlite3_column_blob(stmt, 2),
            messageBytesCount);

        result.emplace_back(
            contractorUUID,
            message,
            messageType);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream CommunicatorMessagesQueueHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream CommunicatorMessagesQueueHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string CommunicatorMessagesQueueHandler::logHeader() const
{
    stringstream s;
    s << "[CommunicatorMessagesQueueHandler]";
    return s.str();
}

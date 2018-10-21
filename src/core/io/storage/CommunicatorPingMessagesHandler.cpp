#include "CommunicatorPingMessagesHandler.h"

CommunicatorPingMessagesHandler::CommunicatorPingMessagesHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger):

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (contractor_uuid BLOB NOT NULL);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorPingMessagesHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("CommunicatorPingMessagesHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void CommunicatorPingMessagesHandler::saveRecord(
    const NodeUUID &contractorUUID)
{
    string query = "INSERT INTO " + mTableName +
                   " (contractor_uuid) VALUES(?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorPingMessagesHandler::insert: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorPingMessagesHandler::insert: "
                          "Bad binding of ContractorUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "inserting is completed successfully";
#endif
    } else {
        throw IOError("CommunicatorPingMessagesHandler::insert: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void CommunicatorPingMessagesHandler::deleteRecord(
    const NodeUUID &contractorUUID)
{
    string query = "DELETE FROM " + mTableName + " WHERE contractor_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorPingMessagesHandler::delete: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorPingMessagesHandler::delete: "
                          "Bad binding of ContractorUUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare deleting is completed successfully";
#endif
    } else {
        throw IOError("CommunicatorPingMessagesHandler::delete: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

vector<NodeUUID> CommunicatorPingMessagesHandler::allContractors()
{
    string queryCount = "SELECT count(*) FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, queryCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorPingMessagesHandler::allMessages: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<NodeUUID> result;
    result.reserve(rowCount);
    string query = "SELECT contractor_uuid FROM " + mTableName + ";";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("CommunicatorPingMessagesHandler::allMessages: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {

        NodeUUID contractorUUID((uint8_t *)sqlite3_column_blob(stmt, 0));
        result.push_back(
            contractorUUID);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream CommunicatorPingMessagesHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream CommunicatorPingMessagesHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string CommunicatorPingMessagesHandler::logHeader() const
{
    stringstream s;
    s << "[CommunicatorPingMessagesHandler]";
    return s.str();
}
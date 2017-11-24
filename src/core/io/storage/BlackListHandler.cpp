#include "BlackListHandler.h"


BlackListHandler::BlackListHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger):

    mLog(logger),
    mDataBase(dbConnection),
    mTableName(tableName)
{
    ensureBlackListTable();
}


void BlackListHandler::ensureBlackListTable()
{
    sqlite3_stmt *stmt;

    /*
     * Main table creation.
     */
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName + " (node_uuid BLOB NOT NULL);";

    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "BlackListHandler::ensureBlackListTable: "
                "Can't create blacklist table: sqlite error code: " + to_string(rc) + ".");
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError(
            "BlackListHandler::ensureBlackListTable: "
                "Can't create blacklist table: sqlite error code: " + to_string(rc) + ".");
    }

    /*
     * Creating unique index for preventing duplicating of migrations UUIDs.
     */
    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName + "_node_uuid_index on " + mTableName + "(node_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "BlackListHandler::ensureBlackListTable: "
                "Can't create index for blacklist table: sqlite error code: " + to_string(rc) + ".");
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError(
            "BlackListHandler::ensureBlackListTable: "
                "Can't create index for blacklist table: sqlite error code: " + to_string(rc) + ".");
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}


vector<NodeUUID> BlackListHandler::allNodesUUIDS() {
    vector<NodeUUID> nodesUUIDs;
    string query = "SELECT node_uuid FROM " + mTableName + ";";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "BlackListHandler::allNodesUUIDS:"
                "Can't select applied  blacklist.  "
                "sqlite error code: " + to_string(rc));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NodeUUID nodeUUID(
            static_cast<const uint8_t *>(
                sqlite3_column_blob(stmt, 0)));

        nodesUUIDs.push_back(nodeUUID);
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return nodesUUIDs;
}


void BlackListHandler::addNode(const NodeUUID &nodeUUID)
{
    string query = "INSERT INTO " + mTableName +
                   "(node_uuid) VALUES (?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("BlackListHandler::insert : "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, nodeUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("BlackListHandler::insert : "
                          "Bad binding of Contractor; sqlite error: "+ to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("BlackListHandler::insert : "
                          "Run query; sqlite error: "+ to_string(rc));
    }
}

const string BlackListHandler::logHeader() const
{
    stringstream s;
    s << "[BlackListHandler]";
    return s.str();
}

LoggerStream BlackListHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream BlackListHandler::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream BlackListHandler::error() const
{
    return mLog.warning(logHeader());
}

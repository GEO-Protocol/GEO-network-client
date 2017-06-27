#include "RoutingTableHandler.h"

RoutingTableHandler::RoutingTableHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                     "(source BLOB NOT NULL, "
                     "destination BLOB NOT NULL);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("RoutingTableHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_source_idx on " + mTableName + "(source);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating index for Source: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("RoutingTableHandler::creating index for Source: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_destination_idx on " + mTableName + "(destination);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating index for Destination: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("RoutingTableHandler::creating index for Destination: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_source_destination_unique_idx ON " + mTableName
            + " (source, destination)";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating unique index for Source and Destination: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("RoutingTableHandler::creating unique index for Source and Destination: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void RoutingTableHandler::deleteRecord(
    const NodeUUID &source,
    const NodeUUID &destination)
{
    string query = "DELETE FROM " + mTableName + " WHERE source = ? AND destination = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::deleteRecord: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, source.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::deleteRecord: "
                          "Bad binding of Source; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::deleteRecord: "
                          "Bad binding of Desitnation; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::deleteRecord: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void RoutingTableHandler::saveRecord(
    const NodeUUID &source,
    const NodeUUID &destination)
{
    string query = "INSERT OR REPLACE INTO " + mTableName +
                   "(source, destination) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, source.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Bad binding of Source; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Bad binding of Desitnation; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting or replacing ("
        << source << ", " << destination << ") " << "is completed successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

vector<pair<NodeUUID, NodeUUID>> RoutingTableHandler::routeRecords()
{
    DateTime startTime = utc_now();
    string countQuery = "SELECT count(*) FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, countQuery.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecords: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<pair<NodeUUID, NodeUUID>> result;
    result.reserve(rowCount);
    string query = "SELECT source, destination FROM " + mTableName;
    rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecords: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 1));
        result.push_back(
            make_pair(
                source,
                destination));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    /*Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecords finished with time: " << methodTime;*/
    return result;
}

unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> RoutingTableHandler::routeRecordsMapDestinationKey()
{
    DateTime startTime = utc_now();
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> result;
    string query = "SELECT source, destination FROM " + mTableName + " ORDER BY destination";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    NodeUUID currentDestination;
    vector<NodeUUID> valueSources;
    if (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 1));
        valueSources.push_back(source);
        currentDestination = destination;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 1));
        if (destination != currentDestination) {
            result.insert(
                make_pair(
                    currentDestination,
                    valueSources));
            valueSources.clear();
            currentDestination = destination;
        }
        valueSources.push_back(source);
    }
    if (valueSources.size() > 0) {
        result.insert(
            make_pair(
                currentDestination,
                valueSources));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    /*Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKey finished with time: " << methodTime;*/
    return result;
}

set<NodeUUID> RoutingTableHandler::neighborsOf (
    const NodeUUID &sourceUUID)
{
    DateTime startTime = utc_now();
    set<NodeUUID> result;
    string query = "SELECT destination FROM " + mTableName + " WHERE source = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::neighborsOf: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, sourceUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::neighborsOf: "
                          "Bad Source binding; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 0));
        result.insert(destination);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    /*Duration methodTime = utc_now() - startTime;
    info() << "neighborsOf method time: " << methodTime;*/
    return result;
}

vector<NodeUUID> RoutingTableHandler::allSourcesForDestination(
    const NodeUUID &destination)
{
    vector<NodeUUID> result;
    string query = "SELECT source FROM " + mTableName + " WHERE destination = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::allSourcesForDestination: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::allSourcesForDestination: "
                          "Bad Destination binding; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        result.push_back(source);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

map<const NodeUUID, vector<NodeUUID>> RoutingTableHandler::routeRecordsMapSourceKey()
{
    map<const NodeUUID, vector<NodeUUID>> result;
    string query = "SELECT source, destination FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapSourceKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 1));
        auto mapElement = result.find(source);
        if (mapElement == result.end()) {
            vector<NodeUUID> newValue;
            newValue.push_back(
                destination);
            result.insert(
                make_pair(
                    source,
                    newValue));
        } else {
            mapElement->second.push_back(
                destination);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

void RoutingTableHandler::deleteAllRecordsWithSource(
    const NodeUUID &sourceUUID)
{
    string query = "DELETE FROM " + mTableName + " WHERE source = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::deleteAllRecordsWithSource: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, sourceUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::deleteAllRecordsWithSource: "
                          "Bad sourceUUID binding; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        info() << "deleting error: " << rc;
        throw IOError("RoutingTableHandler::deleteAllRecordsWithSource: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

const string& RoutingTableHandler::tableName() const
{
    return mTableName;
}

LoggerStream RoutingTableHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream RoutingTableHandler::error() const
{
    return mLog.error(logHeader());
}

const string RoutingTableHandler::logHeader() const
{
    stringstream s;
    s << "[RoutingTableHandler]";
    return s.str();
}
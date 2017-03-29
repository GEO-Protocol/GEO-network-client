#include "RoutingTablesHandler.h"

RoutingTablesHandler::RoutingTablesHandler(
    sqlite3 *db,
    Logger *logger):

    mDataBase(db),
    mRoutingTable2Level(db, kRT2TableName, logger),
    mRoutingTable3Level(db, kRT3TableName, logger),
    mLog(logger){}

RoutingTableHandler* RoutingTablesHandler::routingTable2Level() {

    return &mRoutingTable2Level;
}

RoutingTableHandler* RoutingTablesHandler::routingTable3Level() {

    return &mRoutingTable3Level;
}

vector<NodeUUID> RoutingTablesHandler::subRoutesSecondLevel(
    const NodeUUID &contractorUUID) {

    vector<NodeUUID> result;
    sqlite3_stmt *stmt;
    string query = "SELECT source FROM "
                   + mRoutingTable2Level.tableName() +
                   " WHERE destination = ?";
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "select: " << query;
#endif
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesSecondLevel: "
                              "Bad query " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesSecondLevel: "
                              "Bad binding " + string(sqlite3_errmsg(mDataBase)));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source;
        memcpy(
            source.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        result.push_back(
            source);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<pair<NodeUUID, NodeUUID>> RoutingTablesHandler::subRoutesThirdLevel(
    const NodeUUID &contractorUUID) {

    vector<pair<NodeUUID, NodeUUID>> result;
    sqlite3_stmt *stmt;
    string query = "SELECT rt2.source, rt3.source FROM "
                   + mRoutingTable2Level.tableName() + " AS rt2 INNER JOIN "
                   + mRoutingTable3Level.tableName() + " AS rt3 ON rt2.destination = rt3.source " +
                   " WHERE rt3.destination = ?";
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "select: " << query;
#endif
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad query " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad binding " + string(sqlite3_errmsg(mDataBase)));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source2Level;
        memcpy(
            source2Level.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        NodeUUID source3Level;
        memcpy(
            source3Level.data,
            sqlite3_column_blob(stmt, 1),
            NodeUUID::kBytesSize);
        result.push_back(
            make_pair(
                source2Level,
                source3Level));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream RoutingTablesHandler::info() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream RoutingTablesHandler::error() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string RoutingTablesHandler::logHeader() const {

    stringstream s;
    s << "[RoutingTablesHandler]";
    return s.str();
}
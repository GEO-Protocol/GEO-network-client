#include "RoutingTablesHandler.h"

RoutingTablesHandler::RoutingTablesHandler(
    sqlite3 *db,
    const string &dataBasePath,
    const string &rt2TableName,
    const string &rt3TableName,
    Logger *logger):

    mRoutingTable2Level(db, dataBasePath, rt2TableName, logger),
    mRoutingTable3Level(db, dataBasePath, rt3TableName, logger),
    mLog(logger),
    mDataBase(db) {

    /*int rc = sqlite3_open_v2(dataBasePath.c_str(), &mDataBase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc == SQLITE_OK) {
    } else {
        throw IOError("RoutingTablesHandler::connection "
                          "Can't open database " + dataBasePath);
    }
    info() << "connect";*/
}

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
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesSecondLevel: "
                              "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesSecondLevel: "
                              "Bad Destination binding");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source((uint8_t*)sqlite3_column_blob(stmt, 0));
        result.push_back(
            source);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<pair<NodeUUID, NodeUUID>> RoutingTablesHandler::subRoutesThirdLevelContractor(
        const NodeUUID &contractorUUID,
        const NodeUUID &sourceUUID) {

    vector<pair<NodeUUID, NodeUUID>> result;
    sqlite3_stmt *stmt;
    // TODO (mc) : need or not compare rt3.source with sourceUUID and rt2.source with contractorUUID
    string query = "SELECT rt2.source, rt3.source FROM "
                   + mRoutingTable2Level.tableName() + " AS rt2 INNER JOIN "
                   + mRoutingTable3Level.tableName() + " AS rt3 ON rt2.destination = rt3.source " +
                   " WHERE rt3.destination = ? AND rt2.source <> ? "
                   + "AND rt3.source <> ?";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad foundUUID binding");
    }
    rc = sqlite3_bind_blob(stmt, 2, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad contractorUUID binding");
    }
    rc = sqlite3_bind_blob(stmt, 3, sourceUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad sourceUUID binding");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source2Level((uint8_t*)sqlite3_column_blob(stmt, 0));
        NodeUUID source3Level((uint8_t*)sqlite3_column_blob(stmt, 1));
        result.push_back(
            make_pair(
                source2Level,
                source3Level));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<pair<NodeUUID, NodeUUID>> RoutingTablesHandler::subRoutesThirdLevel(
    const NodeUUID &foundUUID) {

    vector<pair<NodeUUID, NodeUUID>> result;
    sqlite3_stmt *stmt;
    string query = "SELECT rt2.source, rt3.source FROM "
                   + mRoutingTable2Level.tableName() + " AS rt2 INNER JOIN "
                   + mRoutingTable3Level.tableName() + " AS rt3 ON rt2.destination = rt3.source " +
                   " WHERE rt3.destination = ?";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, foundUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad foundUUID binding");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source2Level((uint8_t*)sqlite3_column_blob(stmt, 0));
        NodeUUID source3Level((uint8_t*)sqlite3_column_blob(stmt, 1));
        result.push_back(
            make_pair(
                source2Level,
                source3Level));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<pair<NodeUUID, NodeUUID>> RoutingTablesHandler::subRoutesThirdLevelWithForbiddenNodes(
        const NodeUUID &foundUUID,
        const NodeUUID &sourceUUID,
        const NodeUUID &contractorUUID) {

    vector<pair<NodeUUID, NodeUUID>> result;
    sqlite3_stmt *stmt;
    // TODO (mc) : need or not compare rt3.source with sourceUUID
    string query = "SELECT rt2.source, rt3.source FROM "
                   + mRoutingTable2Level.tableName() + " AS rt2 INNER JOIN "
                   + mRoutingTable3Level.tableName() + " AS rt3 ON rt2.destination = rt3.source " +
                   " WHERE rt3.destination = ? AND rt2.source NOT IN (?, ?) "
                   + "AND rt3.source NOT IN (?, ?)";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, foundUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad foundUUID binding");
    }
    rc = sqlite3_bind_blob(stmt, 2, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad contractorUUID binding");
    }
    rc = sqlite3_bind_blob(stmt, 3, foundUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad foundUUID binding");
    }
    rc = sqlite3_bind_blob(stmt, 4, sourceUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad sourceUUID binding");
    }
    rc = sqlite3_bind_blob(stmt, 5, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::subRoutesThirdLevel: "
                              "Bad sourceUUID binding");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source2Level((uint8_t*)sqlite3_column_blob(stmt, 0));
        NodeUUID source3Level((uint8_t*)sqlite3_column_blob(stmt, 1));
        result.push_back(
            make_pair(
                source2Level,
                source3Level));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

void RoutingTablesHandler::closeConnections() {

    mRoutingTable2Level.closeConnection();
    mRoutingTable3Level.closeConnection();
    sqlite3_close_v2(mDataBase);
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
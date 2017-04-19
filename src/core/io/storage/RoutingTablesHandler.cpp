#include "RoutingTablesHandler.h"

RoutingTablesHandler::RoutingTablesHandler(
    sqlite3 *dbConnection,
    const string &rt2TableName,
    const string &rt3TableName,
    Logger *logger):

    mDataBase(dbConnection),
    mRoutingTable2Level(dbConnection, rt2TableName, logger),
    mRoutingTable3Level(dbConnection, rt3TableName, logger),
    mLog(logger),
    isTransactionBegin(false) {}

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

bool RoutingTablesHandler::commit() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call commit, but trunsaction wasn't started";
#endif
        return true;
    }

    string query = "COMMIT TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTablesHandler::commit: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction commit";
#endif
        isTransactionBegin = false;
        return true;
    } else if (rc == SQLITE_BUSY) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "database busy";
#endif
        return false;
    } else {
        error() << "commit error: " << rc;
        throw IOError("RoutingTablesHandler::commit: "
                          "Run query");
    }
}

void RoutingTablesHandler::rollBack() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call rollBack, but trunsaction wasn't started";
#endif
        return;
    }

    string query = "ROLLBACK TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTablesHandler::rollback: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("RoutingTablesHandler::rollback: "
                          "Run query");
    }

    isTransactionBegin = false;
}

void RoutingTablesHandler::prepareInserted() {

    if (isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call prepareInsertred, but previous transaction isn't finished";
#endif
        return;
    }

    string query = "BEGIN TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTablesHandler::prepareInserted: "
                          "Bad query");
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction begin";
#endif
    } else {
        throw IOError("RoutingTablesHandler::prepareInserted: "
                          "Run query");
    }
    isTransactionBegin = true;
}

void RoutingTablesHandler::saveRecordToRT2(
    const NodeUUID &source,
    const NodeUUID &destination) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    mRoutingTable2Level.saveRecord(
        source,
        destination);
}

void RoutingTablesHandler::saveRecordToRT3(
    const NodeUUID &source,
    const NodeUUID &destination) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    mRoutingTable3Level.saveRecord(
        source,
        destination);
}

void RoutingTablesHandler::deleteRecordFromRT2(
    const NodeUUID &source,
    const NodeUUID &destination) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    mRoutingTable2Level.deleteRecord(
        source,
        destination);

    if (!mRoutingTable2Level.isNodePresentAsDestination(
        destination)) {
        mRoutingTable3Level.deleteAllRecordsWithSource(
            destination);
    }
}

void RoutingTablesHandler::deleteRecordFromRT3(
    const NodeUUID &source,
    const NodeUUID &destination) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    mRoutingTable3Level.deleteRecord(
        source,
        destination);
}

vector<pair<NodeUUID, NodeUUID>> RoutingTablesHandler::rt2Records() {

    return mRoutingTable2Level.routeRecords();
}

vector<pair<NodeUUID, NodeUUID>> RoutingTablesHandler::rt3Records() {

    return mRoutingTable3Level.routeRecords();
}

set<NodeUUID> RoutingTablesHandler::neighborsOfOnRT2(
    const NodeUUID &sourceUUID) {

    return mRoutingTable2Level.neighborsOf(
        sourceUUID);
}

set<NodeUUID> RoutingTablesHandler::neighborsOfOnRT3(
    const NodeUUID &sourceUUID) {

    return mRoutingTable3Level.neighborsOf(
        sourceUUID);
}

unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> RoutingTablesHandler::routeRecordsMapDestinationKeyOnRT2() {

    return mRoutingTable2Level.routeRecordsMapDestinationKey();
}

unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> RoutingTablesHandler::routeRecordsMapDestinationKeyOnRT3() {

    return mRoutingTable3Level.routeRecordsMapDestinationKey();
}

map<const NodeUUID, vector<NodeUUID>> RoutingTablesHandler::routeRecordsMapSourceKeyOnRT2() {

    return mRoutingTable2Level.routeRecordsMapSourceKey();
}

map<const NodeUUID, vector<NodeUUID>> RoutingTablesHandler::routeRecordsMapSourceKeyOnRT3() {

    return mRoutingTable3Level.routeRecordsMapSourceKey();
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
#include "RoutingTableHandler.h"

RoutingTableHandler::RoutingTableHandler(
    sqlite3 *db,
    string tableName,
    Logger *logger) :

    mDataBase(db),
    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false) {

    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                     "(source BLOB NOT NULL, "
                     "destination BLOB NOT NULL, "
                     "direction BLOB);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating table: " + mTableName +
                              " : Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("RoutingTableHandler::creating table: " + mTableName +
                              " : Run query");
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_source_idx on " + mTableName + "(source);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating index for Source: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("RoutingTableHandler::creating index for Source: "
                              "Run query");
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_destination_idx on " + mTableName + "(destination);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating index for Destination: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("RoutingTableHandler::creating index for Destination: "
                              "Run query");
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_source_destination_unique_idx ON " + mTableName
            + " (source, destination)";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating unique index for Source and Destination: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("RoutingTableHandler::creating unique index for Source and Destination: "
                              "Run query");
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void RoutingTableHandler::insert(
        const NodeUUID &source,
        const NodeUUID &destination,
        const TrustLineDirection direction) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "INSERT INTO " + mTableName +
                     "(source, destination, direction) VALUES (?, ?, ?);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, source.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad binding of Source");
    }
    rc = sqlite3_bind_blob(stmt, 2, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad binding of Desitnation");
    }
    switch (direction) {
        case TrustLineDirection::Incoming:
            rc = sqlite3_bind_blob(stmt, 3, "I", 1, SQLITE_STATIC);
            break;
        case TrustLineDirection::Outgoing:
            rc = sqlite3_bind_blob(stmt, 3, "O", 1, SQLITE_STATIC);
            break;
        case TrustLineDirection::Both:
            rc = sqlite3_bind_blob(stmt, 3, "B", 1, SQLITE_STATIC);
            break;
        default:
            throw ValueError("RoutingTableHandler::insert: "
                                     "Direction should be Incomeng, Outgoing or Both");
        }
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad binding of Direction");
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting (" << source << ", " << destination << ") " << "is completed successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::insert: "
                              "Run query");
    }
}

void RoutingTableHandler::commit() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call commit, but trunsaction wasn't started";
#endif
        return;
    }

    string query = "END TRANSACTION;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::commit: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction commit";
#endif
    } else {
        throw IOError("RoutingTableHandler::commit: "
                              "Run query");
    }

    sqlite3_reset(stmt);
    isTransactionBegin = false;
}

void RoutingTableHandler::rollBack() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call rollBack, but trunsaction wasn't started";
#endif
        return;
    }

    sqlite3_finalize(stmt);
    string query = "ROLLBACK;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::rollback: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("RoutingTableHandler::rollback: "
                              "Run query");
    }

    sqlite3_reset(stmt);
    isTransactionBegin = false;
}

void RoutingTableHandler::prepareInserted() {

    if (isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call prepareInsertred, but previous transaction isn't finished";
#endif
        return;
    }

    sqlite3_finalize(stmt);
    string query = "BEGIN TRANSACTION;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::prepareInserted: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction begin";
#endif
    } else {
        throw IOError("RoutingTableHandler::prepareInserted: "
                              "Run query");
    }
    isTransactionBegin = true;
}

void RoutingTableHandler::deleteRecord(
    const NodeUUID &source,
    const NodeUUID &destination) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "DELETE FROM " + mTableName + " WHERE source = ? AND destination = ?;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::deleteRecord: Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, source.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::deleteRecord: "
                          "Bad binding of Source");
    }

    rc = sqlite3_bind_blob(stmt, 2, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::deleteRecord: "
                          "Bad binding of Desitnation");
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::deleteRecord: Run query");
    }
}

void RoutingTableHandler::updateRecord(
    const NodeUUID &source,
    const NodeUUID &destination,
    const TrustLineDirection direction) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "UPDATE " + mTableName + " SET direction = ? WHERE source = ? AND destination = ?;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::updateRecord: Bad query");
    }

    switch (direction) {
        case TrustLineDirection::Incoming:
            rc = sqlite3_bind_blob(stmt, 1, "I", 1, SQLITE_STATIC);
            break;
        case TrustLineDirection::Outgoing:
            rc = sqlite3_bind_blob(stmt, 1, "O", 1, SQLITE_STATIC);
            break;
        case TrustLineDirection::Both:
            rc = sqlite3_bind_blob(stmt, 1, "B", 1, SQLITE_STATIC);
            break;
        default:
            throw ValueError("RoutingTableHandler::updateRecord: "
                                 "Direction should be Incomeng, Outgoing or Both");
    }
    if (rc != SQLITE_OK) {
        throw  IOError("RoutingTableHandler::updateRecord: "
                           "Bad binding of Direction");
    }

    rc = sqlite3_bind_blob(stmt, 2, source.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw  IOError("RoutingTableHandler::updateRecord: "
                           "Bad binding of Source");
    }

    rc = sqlite3_bind_blob(stmt, 3, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw  IOError("RoutingTableHandler::updateRecord: "
                           "Bad binding of Destination");
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "updating is completed successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::updateRecord: Run query");
    }
}

void RoutingTableHandler::saveRecord(
    const NodeUUID &source,
    const NodeUUID &destination,
    const TrustLineDirection direction) {

    if (!isTransactionBegin) {
        prepareInserted();
    }

    string query = "INSERT OR REPLACE INTO " + mTableName +
                   "(source, destination, direction) VALUES (?, ?, ?);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, source.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Bad binding of Source");
    }
    rc = sqlite3_bind_blob(stmt, 2, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Bad binding of Desitnation");
    }
    switch (direction) {
        case TrustLineDirection::Incoming:
            rc = sqlite3_bind_blob(stmt, 3, "I", 1, SQLITE_STATIC);
            break;
        case TrustLineDirection::Outgoing:
            rc = sqlite3_bind_blob(stmt, 3, "O", 1, SQLITE_STATIC);
            break;
        case TrustLineDirection::Both:
            rc = sqlite3_bind_blob(stmt, 3, "B", 1, SQLITE_STATIC);
            break;
        default:
            throw ValueError("RoutingTableHandler::insert or replace: "
                                 "Direction should be Incomeng, Outgoing or Both");
    }
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Bad binding of Direction");
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting or replacing ("
        << source << ", " << destination << ") " << "is completed successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::insert or replace: "
                          "Run query");
    }
}

vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> RoutingTableHandler::routeRecordsWithDirections() {

    DateTime startTime = utc_now();
    /*string countQuery = "SELECT count(*) FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, countQuery.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithDirections: "
                          "Bad count query");
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);*/
    //info() << "routeRecordsWithDirections\t count records: " << rowCount;
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> result;
    //result.reserve(rowCount);

    string query = "SELECT source, destination, direction FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithDirections: "
                              "Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 1));
        char* direction = (char*)sqlite3_column_blob(stmt, 2);
        if (strcmp(direction, "I") == 0) {
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Incoming));
        } else if (strcmp(direction, "O") == 0) {
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Outgoing));
        } else if (strcmp(direction, "B") == 0){
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Both));
        } else {
#ifdef STORAGE_HANDLER_DEBUG_LOG
            error() << "wrong direction during reading from DB";
#endif
            throw ValueError("RoutingTableHandler::routeRecordsWithDirections: "
                                     "Wrong Direction during reading from DB");
        }
    }
    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsWithDirections finished with time: " << methodTime;
    return result;
}

vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> RoutingTableHandler::routeRecordsWithDirectionsWithReserve() {

    DateTime startTime = utc_now();
    string countQuery = "SELECT count(*) FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, countQuery.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithDirectionsWithReserve: "
                          "Bad count query");
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> result;
    result.reserve(rowCount);

    string query = "SELECT source, destination, direction FROM " + mTableName;
    rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithDirectionsWithReserve: "
                          "Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 1));
        char* direction = (char*)sqlite3_column_blob(stmt, 2);
        if (strcmp(direction, "I") == 0) {
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Incoming));
        } else if (strcmp(direction, "O") == 0) {
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Outgoing));
        } else if (strcmp(direction, "B") == 0){
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Both));
        } else {
#ifdef STORAGE_HANDLER_DEBUG_LOG
            error() << "wrong direction during reading from DB";
#endif
            throw ValueError("RoutingTableHandler::routeRecordsWithDirectionsWithReserve: "
                                 "Wrong Direction during reading from DB");
        }
    }
    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsWithDirectionsWithReserve finished with time: " << methodTime;
    return result;
}

vector<pair<NodeUUID, NodeUUID>> RoutingTableHandler::routeRecords() {

    DateTime startTime = utc_now();
    vector<pair<NodeUUID, NodeUUID>> result;
    string query = "SELECT source, destination FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecords: "
                              "Bad query");
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
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecords finished with time: " << methodTime;
    return result;
}

vector<pair<NodeUUID, NodeUUID>> RoutingTableHandler::routeRecordsWithResesrve() {

    DateTime startTime = utc_now();
    string countQuery = "SELECT count(*) FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, countQuery.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithReserve: "
                          "Bad count query");
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    vector<pair<NodeUUID, NodeUUID>> result;
    result.reserve(rowCount);
    string query = "SELECT source, destination FROM " + mTableName;
    rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithReserve: "
                          "Bad query");
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
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsWithReserve finished with time: " << methodTime;
    return result;
}

unordered_map<NodeUUID, vector<NodeUUID>> RoutingTableHandler::routeRecordsMapDestinationKey() {

    DateTime startTime = utc_now();
    unordered_map<NodeUUID, vector<NodeUUID>> result;
    string query = "SELECT source, destination FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKey: "
                              "Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 1));
        auto element = result.find(destination);
        if (element == result.end()) {
            vector<NodeUUID> newVect;
            newVect.push_back(source);
            result.insert(
                make_pair(
                    destination,
                    newVect));
        } else {
            element->second.push_back(source);
        }

    }
    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKey finished with time: " << methodTime;
    return result;
}

unordered_map<NodeUUID, vector<NodeUUID>> RoutingTableHandler::routeRecordsMapDestinationKeyOpt() {

    DateTime startTime = utc_now();
    unordered_map<NodeUUID, vector<NodeUUID>> result;
    string queryKey = "SELECT DISTINCT destination FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, queryKey.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt select keys: "
                          "Bad query");
    }
    vector<NodeUUID> keys;
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID keyDestination((uint8_t *)sqlite3_column_blob(stmt, 0));
        keys.push_back(keyDestination);
    }
    for (const auto destination : keys) {
        string query = "SELECT source FROM " + mTableName + " WHERE destination = ?";
        rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt: "
                              "Bad query");
        }
        rc = sqlite3_bind_blob(stmt, 1, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt: "
                              "Bad Destination binding");
        }
        vector<NodeUUID> newVect;
        while (sqlite3_step(stmt) == SQLITE_ROW ) {
            NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
            newVect.push_back(source);
        }
        result.insert(
            make_pair(
                destination,
                newVect));
    }

    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKeyOpt finished with time: " << methodTime;
    return result;
}

unordered_map<NodeUUID, vector<NodeUUID>> RoutingTableHandler::routeRecordsMapDestinationKeyOpt1() {

    DateTime startTime = utc_now();
    unordered_map<NodeUUID, vector<NodeUUID>> result;
    string queryKeyCount = "SELECT count(DISTINCT destination) FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, queryKeyCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt1: "
                          "Bad count query");
    }
    sqlite3_step(stmt);
    uint32_t keyCount = (uint32_t)sqlite3_column_int(stmt, 0);

    string queryKey = "SELECT DISTINCT destination FROM " + mTableName;
    rc = sqlite3_prepare_v2( mDataBase, queryKey.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt1 select keys: "
                          "Bad query");
    }
    vector<NodeUUID> keys;
    keys.reserve(keyCount);
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID keyDestination((uint8_t *)sqlite3_column_blob(stmt, 0));
        keys.push_back(keyDestination);
    }
    for (const auto destination : keys) {
        string query = "SELECT source FROM " + mTableName + " WHERE destination = ?";
        rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt1: "
                              "Bad query");
        }
        rc = sqlite3_bind_blob(stmt, 1, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt1: "
                              "Bad Destination binding");
        }
        vector<NodeUUID> newVect;
        while (sqlite3_step(stmt) == SQLITE_ROW ) {
            NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
            newVect.push_back(source);
        }
        result.insert(
            make_pair(
                destination,
                newVect));
    }

    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKeyOpt1 finished with time: " << methodTime;
    return result;
}

unordered_map<NodeUUID, vector<NodeUUID>> RoutingTableHandler::routeRecordsMapDestinationKeyOpt2() {

    DateTime startTime = utc_now();
    unordered_map<NodeUUID, vector<NodeUUID>> result;
    string queryKeyCount = "SELECT count(DISTINCT destination) FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, queryKeyCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt2: "
                          "Bad count key query");
    }
    sqlite3_step(stmt);
    uint32_t keyCount = (uint32_t)sqlite3_column_int(stmt, 0);

    string queryKey = "SELECT DISTINCT destination FROM " + mTableName;
    rc = sqlite3_prepare_v2( mDataBase, queryKey.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt2 select keys: "
                          "Bad query");
    }
    vector<NodeUUID> keys;
    keys.reserve(keyCount);
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID keyDestination((uint8_t *)sqlite3_column_blob(stmt, 0));
        keys.push_back(keyDestination);
    }
    for (const auto destination : keys) {

        string queryRowCount = "SELECT count(source) FROM " + mTableName + " WHERE destination = ?";
        rc = sqlite3_prepare_v2( mDataBase, queryRowCount.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt2: "
                              "Bad count row query");
        }
        sqlite3_step(stmt);
        uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);

        string query = "SELECT source FROM " + mTableName + " WHERE destination = ?";
        rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt2: "
                              "Bad query");
        }
        rc = sqlite3_bind_blob(stmt, 1, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt2: "
                              "Bad Destination binding");
        }
        vector<NodeUUID> newVect;
        newVect.reserve(rowCount);
        while (sqlite3_step(stmt) == SQLITE_ROW ) {
            NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
            newVect.push_back(source);
        }
        result.insert(
            make_pair(
                destination,
                newVect));
    }

    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKeyOpt2 finished with time: " << methodTime;
    return result;
}

unordered_map<NodeUUID, vector<NodeUUID>> RoutingTableHandler::routeRecordsMapDestinationKeyOpt3() {

    DateTime startTime = utc_now();
    unordered_map<NodeUUID, vector<NodeUUID>> result;
    string query = "SELECT source, destination FROM " + mTableName + " ORDER BY destination";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt3: "
                          "Bad query");
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
    result.insert(
        make_pair(
            currentDestination,
            valueSources));
    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKeyOpt3 finished with time: " << methodTime;
    return result;
}

map<NodeUUID, vector<NodeUUID>> RoutingTableHandler::routeRecordsMapDestinationKeyOpt4() {

    DateTime startTime = utc_now();
    map<NodeUUID, vector<NodeUUID>> result;
    string query = "SELECT source, destination FROM " + mTableName + " ORDER BY destination";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt4: "
                          "Bad query");
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
    result.insert(
        make_pair(
            currentDestination,
            valueSources));
    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKeyOpt4 finished with time: " << methodTime;
    return result;
}

unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> RoutingTableHandler::routeRecordsMapDestinationKeyOpt5() {

    DateTime startTime = utc_now();
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> result;
    string query = "SELECT source, destination FROM " + mTableName + " ORDER BY destination";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKeyOpt5: "
                          "Bad query");
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
    result.insert(
        make_pair(
            currentDestination,
            valueSources));
    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKeyOpt5 finished with time: " << methodTime;
    return result;
}

vector<NodeUUID> RoutingTableHandler::allDestinationsForSource(
    const NodeUUID &sourceUUID) {

    DateTime startTime = utc_now();
    string countQuery = "SELECT count(*) FROM " + mTableName + " WHERE source = ?";
    int rc = sqlite3_prepare_v2( mDataBase, countQuery.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::allDestinationsForSource: "
                          "Bad count query");
    }
    rc = sqlite3_bind_blob(stmt, 1, sourceUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::allDestinationsForSource: "
                          "Bad Source binding");
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    //info() << "allDestinationsForSource\t count records: " << rowCount;
    vector<NodeUUID> result;
    result.reserve(rowCount);

    string query = "SELECT destination FROM " + mTableName + " WHERE source = ?";
    rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::allDestinationsForSource: "
                              "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, sourceUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::allDestinationsForSource: "
                              "Bad Source binding");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 0));
        result.push_back(destination);
    }
    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "allDestinationsForSource method time: " << methodTime;
    return result;
}

NodeUUID RoutingTableHandler::sourceMaxCount() {

    string query = "SELECT source, count(destination) AS cnt FROM " + mTableName + " GROUP BY source ORDER BY cnt";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::sourceMaxCount: "
                          "Bad count query");
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        NodeUUID souce((uint8_t *)sqlite3_column_blob(stmt, 0));
        info() << "sourceMaxCount source count: " << sqlite3_column_int(stmt, 1);
        return souce;
    } else {
        return NodeUUID("13e5cf8c-5834-4e52-b65b-000000000000");
    }
}

map<const NodeUUID, vector<pair<const NodeUUID, const TrustLineDirection>>> RoutingTableHandler::routeRecordsWithDirectionsMapSourceKey() {

    map<const NodeUUID, vector<pair<const NodeUUID, const TrustLineDirection>>> result;
    string query = "SELECT source, destination, direction FROM " + mTableName;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithDirectionsMapSourceKey: "
                              "Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NodeUUID source((uint8_t *)sqlite3_column_blob(stmt, 0));
        NodeUUID destination((uint8_t *)sqlite3_column_blob(stmt, 1));
        char* directionChr = (char*)sqlite3_column_blob(stmt, 2);
        TrustLineDirection direction;
        if (strcmp(directionChr, "I") == 0) {
            direction = TrustLineDirection::Incoming;
        } else if (strcmp(directionChr, "O") == 0) {
            direction = TrustLineDirection::Outgoing;
        } else if (strcmp(directionChr, "B") == 0) {
            direction = TrustLineDirection::Both;
        } else {
#ifdef STORAGE_HANDLER_DEBUG_LOG
            error() << "wrong direction during reading from DB";
#endif
            throw ValueError("RoutingTableHandler::routeRecordsWithDirectionsMapSourceKey: "
                                     "Wrong Direction during reading from DB");
        }
        auto mapElement = result.find(source);
        if (mapElement == result.end()) {
            vector<pair<const NodeUUID, const TrustLineDirection>> newValue;
            newValue.push_back(
                make_pair(
                    destination,
                    direction));
            result.insert(
                make_pair(
                    source,
                    newValue));
        } else {
            mapElement->second.push_back(
                make_pair(
                    destination,
                    direction));
        }
    }
    return result;
}

const string& RoutingTableHandler::tableName() const {

    return mTableName;
}

LoggerStream RoutingTableHandler::info() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream RoutingTableHandler::error() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string RoutingTableHandler::logHeader() const {

    stringstream s;
    s << "[RoutingTableHandler]";
    return s.str();
}
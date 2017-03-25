#include "RoutingTableHandler.h"

RoutingTableHandler::RoutingTableHandler(
    sqlite3 *db,
    string tableName,
    Logger *logger) :

    mDataBase(db),
    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false) {

    string query = createTableQuery();
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating table: "
                              "Bad query " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "table" << mTableName << "created successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::creating table: "
                              "Run query " + string(sqlite3_errmsg(mDataBase)));
    }

    query = createIndexQuery("source");
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating index: "
                              "Bad query " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "index for source created successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::creating index: "
                              "Run query " + string(sqlite3_errmsg(mDataBase)));
    }

    query = createIndexQuery("destination");
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating index: "
                              "Bad query " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "index for destination created successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::creating index: "
                              "Run query " + string(sqlite3_errmsg(mDataBase)));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void RoutingTableHandler::insert(
        const NodeUUID &source,
        const NodeUUID &destination,
        const TrustLineDirection direction) {

    string query = insertQuery();
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad query " + string(sqlite3_errmsg(mDataBase)));
    }

    rc = sqlite3_bind_blob(stmt, 1, source.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad binding " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_bind_blob(stmt, 2, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad binding " + string(sqlite3_errmsg(mDataBase)));
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
                              "Bad binding " + string(sqlite3_errmsg(mDataBase)));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "inserting is completed successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::insert: "
                              "Run query " + string(sqlite3_errmsg(mDataBase)));
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
                              "Bad query: " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction commit";
#endif
    } else {
        throw IOError("RoutingTableHandler::commit: "
                              "Run query " + string(sqlite3_errmsg(mDataBase)));
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
                              "Bad query: " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("RoutingTableHandler::rollback: "
                              "Run query " + string(sqlite3_errmsg(mDataBase)));
    }

    sqlite3_reset(stmt);
    isTransactionBegin = false;
}

void RoutingTableHandler::prepareInsertred() {

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
        throw IOError("RoutingTableHandler::prepareInsertred: "
                              "Bad query: " + string(sqlite3_errmsg(mDataBase)));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction begin";
#endif
    } else {
        throw IOError("RoutingTableHandler::prepareInsertred: "
                              "Run query " + string(sqlite3_errmsg(mDataBase)));
    }
    isTransactionBegin = true;

}

vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> RoutingTableHandler::routeRecords() {

    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> result;
    string query = selectQuery();
    info() << "select: " << query;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecords: "
                              "Bad query " + string(sqlite3_errmsg(mDataBase)));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source;
        memcpy(
            source.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        NodeUUID destination;
        memcpy(
            destination.data,
            sqlite3_column_blob(stmt, 1),
            NodeUUID::kBytesSize);
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
            throw ValueError("RoutingTableHandler::routeRecords: "
                                     "Wrong Direction during reading from DB");
        }
    }
    sqlite3_reset(stmt);
    return result;
}

const string RoutingTableHandler::createTableQuery() const {

    stringstream s;
    s << "CREATE TABLE IF NOT EXISTS "  << mTableName.c_str() <<
            "(source BLOB NOT NULL, "
            "destination BLOB NOT NULL, "
            "direction BLOB NOT NULL);";
    return s.str();
}

const string RoutingTableHandler::createIndexQuery(string fieldName) const {

    stringstream s;
    s << "CREATE INDEX IF NOT EXISTS "<< mTableName.c_str() <<
            "_" << fieldName.c_str() << "_idx on " <<
            mTableName.c_str() << "(" << fieldName.c_str() << ");";
    return s.str();
}

const string RoutingTableHandler::insertQuery() const {

    stringstream s;
    s << "INSERT INTO " << mTableName.c_str() <<
            "(source, destination, direction) VALUES (?, ?, ?);";
    return s.str();
}

const string RoutingTableHandler::selectQuery() const {

    info() << "selecting:";
    stringstream s;
    s <<  "SELECT source, destination, direction FROM " << mTableName.c_str();
    return s.str();
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


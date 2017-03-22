#include "RoutingTableHandler.h"

RoutingTableHandler::RoutingTableHandler(
    sqlite3 *db,
    string tableName,
    Logger *logger) :

    mDataBase(db),
    mTableName(tableName),
    mLog(logger) {

    info() << "creating table " << mTableName;
    string query = createTableQuery();
    info() << "query " << query;
    int rc = rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        error() << "Can't create table " << mTableName << " : " << sqlite3_errmsg(mDataBase);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        info() << "table created successfully";
    } else {
        error() << "table creating execution failed: " << sqlite3_errmsg(db) << endl;
    }

    info() << "creating left node index for " << mTableName;
    query = createIndexQuery("left_node");
    info() << "query " << query;
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        error() << "Can't create index for left_node: " << sqlite3_errmsg(mDataBase);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        info() << "index for left node created successfully";
    } else {
        error() << "left node index creating execution failed: " << sqlite3_errmsg(db) << endl;
    }

    info() << "creating right node index for " << mTableName;
    query = createIndexQuery("right_node");
    info() << "query " << query;
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        error() << "Can't create index for right_node: " << sqlite3_errmsg(mDataBase);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        info() << "index for right node created successfully";
    } else {
        error() << "right node index creating execution failed: " << sqlite3_errmsg(mDataBase) << endl;
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void RoutingTableHandler::insert(
        const NodeUUID &leftNode,
        const NodeUUID &rightNode,
        DirectionType direction) {

    info() << "inserting";
    string query = insertQuery();
    info() << "query " << query;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        error() << "Can't insert: " << sqlite3_errmsg(mDataBase);
    }
    rc = sqlite3_bind_blob( stmt, 1, leftNode.data, -1, 0 );
    if (rc != SQLITE_OK) {
        error() << "bind left node failed: " << sqlite3_errmsg(mDataBase);
    }
    rc = sqlite3_bind_blob(stmt, 2, rightNode.data, 16, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        error() << "bind right node failed: " << sqlite3_errmsg(mDataBase);
    }
    switch (direction) {
        case DirectionType::Incoming:
            rc = sqlite3_bind_blob(stmt, 3, "I", 1, SQLITE_STATIC);
            break;
        case DirectionType::Outgoing:
            rc = sqlite3_bind_blob(stmt, 3, "O", 1, SQLITE_STATIC);
            break;
        case DirectionType::Both:
            rc = sqlite3_bind_blob(stmt, 3, "B", 1, SQLITE_STATIC);
            break;
    }
    if (rc != SQLITE_OK) {
        error() << "bind direction failed: " << sqlite3_errmsg(mDataBase);
    }
}

void RoutingTableHandler::commit() {

    info() << "commit";
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        info() << "inserting is completed successfully";
    } else {
        error() << "inserting execution failed: " << sqlite3_errmsg(mDataBase) << endl;
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void RoutingTableHandler::rollBack() {

    info() << "rollback";
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void RoutingTableHandler::prepareInsertred() {

    info() << "prepare inserting";
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

const string RoutingTableHandler::createTableQuery() const {

    stringstream s;
    s << "CREATE TABLE IF NOT EXISTS "  << mTableName.c_str() <<
            "(left_node BLOB NOT NULL, "
            "right_node BLOB NOT NULL, "
            "direction BLOB NOT NULL)";
    return s.str();
}

const string RoutingTableHandler::createIndexQuery(string fieldName) const {

    stringstream s;
    s << "CREATE INDEX IF NOT EXISTS "<< mTableName.c_str() <<
            "_" << fieldName.c_str() << "_idx on " <<
            mTableName.c_str() << "(" << fieldName.c_str() << ")";
    return s.str();
}

const string RoutingTableHandler::insertQuery() const {

    stringstream s;
    s << "INSERT INTO " << mTableName.c_str() <<
            "(left_node, right_node, direction) VALUES (?, ?, ?)";
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


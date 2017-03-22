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
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
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

    mLeftNodes.push_back(leftNode);
    mRightNodes.push_back(rightNode);
    mDirections.push_back(direction);
}

void RoutingTableHandler::commit() {

    info() << "commit";

    if (mLeftNodes.size() == 0) {
        info() << "no data for inserting";
        return;
    }

    string query = insertHeaderQuery();
    for (int idx = 0; idx < mLeftNodes.size() - 1; idx++) {
        query = query + insertBodyQuery() + ",";
    }
    query = query + insertBodyQuery() + ";";
    info() << "query: " << query;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        error() << "Bad insert query: " << sqlite3_errmsg(mDataBase);
    }

    int idxParam = 1;
    for (uint32_t idx = 0; idx < mLeftNodes.size(); idx++) {
        rc = sqlite3_bind_blob(stmt, idxParam++, mLeftNodes.at(idx).data, NodeUUID::kBytesSize, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            error() << "bind left node failed: " << sqlite3_errmsg(mDataBase);
        }
        rc = sqlite3_bind_blob(stmt, idxParam++, mRightNodes.at(idx).data, NodeUUID::kBytesSize, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            error() << "bind right node failed: " << sqlite3_errmsg(mDataBase);
        }
        switch (mDirections.at(idx)) {
            case DirectionType::Incoming:
                rc = sqlite3_bind_blob(stmt, idxParam++, "I", 1, SQLITE_STATIC);
                break;
            case DirectionType::Outgoing:
                rc = sqlite3_bind_blob(stmt, idxParam++, "O", 1, SQLITE_STATIC);
                break;
            case DirectionType::Both:
                rc = sqlite3_bind_blob(stmt, idxParam++, "B", 1, SQLITE_STATIC);
                break;
        }
        if (rc != SQLITE_OK) {
            error() << "bind direction failed: " << sqlite3_errmsg(mDataBase);
        }
    }


    rc = sqlite3_step(stmt);
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
    mLeftNodes.clear();
    mRightNodes.clear();
    mDirections.clear();
    //sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void RoutingTableHandler::prepareInsertred() {

    info() << "prepare inserting";
    mLeftNodes.clear();
    mRightNodes.clear();
    mDirections.clear();
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

vector<NodeUUID> RoutingTableHandler::leftNodes() {

    vector<NodeUUID> result;
    info() << "selecting";
    string query = selectQuery();
    info() << "query: " << query;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        error() << "Bad select query: " << sqlite3_errmsg(mDataBase);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID leftNode;
        memcpy(
            leftNode.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        result.push_back(leftNode);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
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

const string RoutingTableHandler::insertHeaderQuery() const {

    stringstream s;
    s << "INSERT INTO " << mTableName.c_str() <<
            "(left_node, right_node, direction) VALUES";
    return s.str();
}

const string RoutingTableHandler::insertBodyQuery() const {

    stringstream s;
    s <<  "(?, ?, ?)";
    return s.str();
}

const string RoutingTableHandler::selectQuery() const {

    stringstream s;
    s <<  "SELECT left_node, right_node, direction FROM " << mTableName.c_str();
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


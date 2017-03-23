#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H

#include "../../common/NodeUUID.h"
#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>
#include <tuple>

class RoutingTableHandler {

public:

    enum DirectionType {
        Incoming = 1,
        Outgoing,
        Both
    };

public:

    RoutingTableHandler(
        sqlite3 *db,
        string tableName,
        Logger *logger);

    void prepareInsertred();

    void commit();

    void rollBack();

    void insert(
        const NodeUUID &source,
        const NodeUUID &destination,
        DirectionType direction);

    vector<tuple<NodeUUID, NodeUUID, RoutingTableHandler::DirectionType>> routeRecords();

private:

    const string createTableQuery() const;

    const string createIndexQuery(string fieldName) const;

    const string insertQuery() const;

    const string selectQuery() const;

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase;
    string mTableName;
    sqlite3_stmt *stmt;
    Logger *mLog;
    bool isTransactionBegin;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H

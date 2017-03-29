#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H

#include "../../common/NodeUUID.h"
#include "../../common/Types.h"
#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>
#include <tuple>
#include <unordered_map>

class RoutingTableHandler {

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
        const TrustLineDirection direction);

    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> routeRecordsWithDirections();

    vector<pair<NodeUUID, NodeUUID>> routeRecords();

    vector<NodeUUID> allDestinationsForSource(
        const NodeUUID &sourceUUID);

    unordered_map<NodeUUID, vector<NodeUUID>> routeRecordsMapDestinationKey();

    const string &tableName() const;

private:

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

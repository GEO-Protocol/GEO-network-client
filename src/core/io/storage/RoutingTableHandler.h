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
#include <boost/functional/hash.hpp>

class RoutingTableHandler {

public:

    RoutingTableHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger *logger);

    void saveRecord(
        const NodeUUID &source,
        const NodeUUID &destination);

    void deleteRecord(
        const NodeUUID &source,
        const NodeUUID &destination);

    vector<pair<NodeUUID, NodeUUID>> routeRecords();

    vector<NodeUUID> allDestinationsForSource(
        const NodeUUID &sourceUUID);

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> routeRecordsMapDestinationKey();

    map<const NodeUUID, vector<NodeUUID>> routeRecordsMapSourceKey();

    bool isNodePresentAsDestination(const NodeUUID &nodeUUID);

    void deleteAllRecordsWithSource(const NodeUUID &sourceUUID);

    const string &tableName() const;

    void closeConnection();

private:

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H

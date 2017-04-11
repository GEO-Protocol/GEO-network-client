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
        sqlite3 *db,
        const string &databasePath,
        const string &tableName,
        Logger *logger);

    void commit();

    void rollBack();

    void saveRecord(
        const NodeUUID &source,
        const NodeUUID &destination,
        const TrustLineDirection direction);

    void deleteRecord(
        const NodeUUID &source,
        const NodeUUID &destination);

    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> routeRecordsWithDirections();

    vector<pair<NodeUUID, NodeUUID>> routeRecords();

    vector<NodeUUID> allDestinationsForSource(
        const NodeUUID &sourceUUID);

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> routeRecordsMapDestinationKey();

    map<const NodeUUID, vector<pair<const NodeUUID, const TrustLineDirection>>> routeRecordsWithDirectionsMapSourceKey();

    const string &tableName() const;

    void closeConnection();

private:

    void prepareInserted();

    void insert(
        const NodeUUID &source,
        const NodeUUID &destination,
        const TrustLineDirection direction);

    void updateRecord(
        const NodeUUID &source,
        const NodeUUID &destination,
        const TrustLineDirection direction);

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

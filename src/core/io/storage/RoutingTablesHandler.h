#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H

#include "RoutingTableHandler.h"
#include "../../logger/Logger.h"
#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>

class RoutingTablesHandler {

public:

    RoutingTablesHandler(
        sqlite3 *dbConnection,
        const string &rt2TableName,
        const string &rt3TableName,
        Logger *logger);

    vector<NodeUUID> subRoutesSecondLevel(const NodeUUID &contractorUUID);

    vector<pair<NodeUUID, NodeUUID>> subRoutesThirdLevelContractor(
            const NodeUUID &contractorUUID,
            const NodeUUID &sourceUUID);

    vector<pair<NodeUUID, NodeUUID>> subRoutesThirdLevel(const NodeUUID &foundUUID);

    vector<pair<NodeUUID, NodeUUID>> subRoutesThirdLevelWithForbiddenNodes(
        const NodeUUID &foundUUID,
        const NodeUUID &sourceUUID,
        const NodeUUID &contractorUUID);

    void saveRecordToRT2(
        const NodeUUID &source,
        const NodeUUID &destination);

    void saveRecordToRT3(
        const NodeUUID &source,
        const NodeUUID &destination);

    void deleteRecordFromRT2(
        const NodeUUID &source,
        const NodeUUID &destination);

    void deleteRecordFromRT3(
        const NodeUUID &source,
        const NodeUUID &destination);

    vector<pair<NodeUUID, NodeUUID>> rt2Records();

    vector<pair<NodeUUID, NodeUUID>> rt3Records();

    set<NodeUUID> neighborsOfOnRT2(
        const NodeUUID &sourceUUID);

    set<NodeUUID> neighborsOfOnRT3(
        const NodeUUID &sourceUUID);

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> routeRecordsMapDestinationKeyOnRT2();

    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> routeRecordsMapDestinationKeyOnRT3();

    map<const NodeUUID, vector<NodeUUID>> routeRecordsMapSourceKeyOnRT2();

    map<const NodeUUID, vector<NodeUUID>> routeRecordsMapSourceKeyOnRT3();

private:
    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    RoutingTableHandler mRoutingTable2Level;
    RoutingTableHandler mRoutingTable3Level;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H

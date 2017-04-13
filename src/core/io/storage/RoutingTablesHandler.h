#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H

#include "RoutingTableHandler.h"
#include "../../logger/Logger.h"
#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>

class RoutingTablesHandler {

public:

    RoutingTablesHandler(
        sqlite3 *db,
        const string &rt2TableName,
        const string &rt3TableName,
        Logger *logger);

    RoutingTableHandler* routingTable2Level();

    RoutingTableHandler* routingTable3Level();

    vector<NodeUUID> subRoutesSecondLevel(const NodeUUID &contractorUUID);

    vector<pair<NodeUUID, NodeUUID>> subRoutesThirdLevelContractor(
            const NodeUUID &contractorUUID,
            const NodeUUID &sourceUUID);

    vector<pair<NodeUUID, NodeUUID>> subRoutesThirdLevel(const NodeUUID &foundUUID);

    vector<pair<NodeUUID, NodeUUID>> subRoutesThirdLevelWithForbiddenNodes(
        const NodeUUID &foundUUID,
        const NodeUUID &sourceUUID,
        const NodeUUID &contractorUUID);

private:

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase;
    RoutingTableHandler mRoutingTable2Level;
    RoutingTableHandler mRoutingTable3Level;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H

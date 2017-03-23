#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H

#include "RoutingTableHandler.h"
#include "../../logger/Logger.h"
#include "../../../libs/sqlite3/sqlite3.h"

class RoutingTablesHandler {

public:

    RoutingTablesHandler(
        sqlite3 *db,
        Logger *logger);

    RoutingTableHandler* routingTable2Level();

    RoutingTableHandler* routingTable3Level();

private:

    const string kRT2TableName = "RT2";

    const string kRT3TableName = "RT3";

private:
    RoutingTableHandler mRoutingTable2Level;
    RoutingTableHandler mRoutingTable3Level;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESHANDLER_H

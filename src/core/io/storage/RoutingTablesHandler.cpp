#include "RoutingTablesHandler.h"

RoutingTablesHandler::RoutingTablesHandler(
    sqlite3 *db,
    Logger *logger):
    mLog(logger){

    mRoutingTable2Level = new RoutingTableHandler(
        db,
        kRT2TableName,
        mLog);
    mRoutingTable3Level = new RoutingTableHandler(
        db,
        kRT3TableName,
        mLog);
}

RoutingTablesHandler::~RoutingTablesHandler() {

    delete mRoutingTable2Level;
    delete mRoutingTable3Level;
}

RoutingTableHandler* RoutingTablesHandler::routingTable2Level() const {

    return mRoutingTable2Level;
}

RoutingTableHandler* RoutingTablesHandler::routingTable3Level() const {

    return mRoutingTable3Level;
}
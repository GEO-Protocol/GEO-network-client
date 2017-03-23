#include "RoutingTablesHandler.h"

RoutingTablesHandler::RoutingTablesHandler(
    sqlite3 *db,
    Logger *logger):
    mRoutingTable2Level(db, kRT2TableName, logger),
    mRoutingTable3Level(db, kRT3TableName, logger),
    mLog(logger){}

RoutingTableHandler* RoutingTablesHandler::routingTable2Level() {

    return &mRoutingTable2Level;
}

RoutingTableHandler* RoutingTablesHandler::routingTable3Level() {

    return &mRoutingTable3Level;
}
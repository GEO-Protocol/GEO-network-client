#ifndef GEO_NETWORK_CLIENT_STORAGEHANDLER_H
#define GEO_NETWORK_CLIENT_STORAGEHANDLER_H

#include "../../logger/Logger.h"
#include "RoutingTablesHandler.h"
#include "RoutingTableHandler.h"
#include "../../../libs/sqlite3/sqlite3.h"

class StorageHandler {

public:

    StorageHandler(Logger *logger);

    ~StorageHandler();

    void insertRT2(
        const NodeUUID &leftNode,
        const NodeUUID &rightNode,
        RoutingTableHandler::DirectionType direction) const;

    void insertRT3(
        const NodeUUID &leftNode,
        const NodeUUID &rightNode,
        RoutingTableHandler::DirectionType direction) const;

    void commit() const;

    void rollback() const;

    void prepareInsertred() const;

    RoutingTablesHandler *routingTablesHandler() const;

private:

    void openConnection();

    void closeConnection();

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    const char *kDataBaseName = "myDB";

private:
    sqlite3 *mDataBase;
    Logger *mLog;
    RoutingTablesHandler *mRoutingTablesHandler;

};


#endif //GEO_NETWORK_CLIENT_STORAGEHANDLER_H

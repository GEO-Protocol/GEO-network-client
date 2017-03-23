#ifndef GEO_NETWORK_CLIENT_STORAGEHANDLER_H
#define GEO_NETWORK_CLIENT_STORAGEHANDLER_H

#include "../../logger/Logger.h"
#include "RoutingTablesHandler.h"
#include "RoutingTableHandler.h"
#include "../../../libs/sqlite3/sqlite3.h"

#include <boost/filesystem.hpp>
#include <vector>

namespace fs = boost::filesystem;

class StorageHandler {

public:

    StorageHandler(
        const string &directory,
        const string &dataBaseName,
        Logger *logger);

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

    vector<NodeUUID> leftNodesRT2() const;

    vector<NodeUUID> leftNodesRT3() const;

private:

    void checkDirectory();

    void openConnection();

    void closeConnection();

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    const char *kDataBaseName = "io/storage_handler/myDB";

private:
    sqlite3 *mDataBase;
    Logger *mLog;
    RoutingTablesHandler *mRoutingTablesHandler;
    string mDirectory;
    string mDataBaseName;
    string mDataBasePath;

};


#endif //GEO_NETWORK_CLIENT_STORAGEHANDLER_H

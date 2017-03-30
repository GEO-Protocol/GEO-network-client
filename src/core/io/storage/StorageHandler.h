#ifndef GEO_NETWORK_CLIENT_STORAGEHANDLER_H
#define GEO_NETWORK_CLIENT_STORAGEHANDLER_H

#include "../../logger/Logger.h"
#include "RoutingTablesHandler.h"
#include "RoutingTableHandler.h"
#include "../../common/exceptions/IOError.h"
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

    RoutingTablesHandler* routingTablesHandler();

    void closeConnection();

private:

    static void checkDirectory(
        const string &directory);

    static sqlite3* connection(
        const string &dataBaseName,
        const string &directory);

    LoggerStream info() const;

    const string logHeader() const;

private:

    static sqlite3 *mDataBase;

private:

    Logger *mLog;
    RoutingTablesHandler mRoutingTablesHandler;
    string mDirectory;
    string mDataBaseName;
    string mDataBasePath;

};


#endif //GEO_NETWORK_CLIENT_STORAGEHANDLER_H

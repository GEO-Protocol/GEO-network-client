#ifndef GEO_NETWORK_CLIENT_PATHSMANAGER_H
#define GEO_NETWORK_CLIENT_PATHSMANAGER_H

#include "../trust_lines/manager/TrustLinesManager.h"
#include "../io/storage/StorageHandler.h"
#include "../logger/Logger.h"

#include <vector>
#include <set>
#include <tuple>

class PathsManager {

public:

    PathsManager(
        TrustLinesManager *trustLinesManager,
        StorageHandler* storageHandler,
        Logger *logger);

private:

    void loadSelfRoutingTables();

    void removeUselessRecords();

    LoggerStream info() const;

    const string logHeader() const;

private:

    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    Logger *mLog;
    vector<pair<const NodeUUID, const TrustLineDirection>> mRT1;
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> mRT2;
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> mRT3;

};


#endif //GEO_NETWORK_CLIENT_PATHSMANAGER_H

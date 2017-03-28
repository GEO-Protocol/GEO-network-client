#ifndef GEO_NETWORK_CLIENT_PATHSMANAGER_H
#define GEO_NETWORK_CLIENT_PATHSMANAGER_H

#include "../trust_lines/manager/TrustLinesManager.h"
#include "../network/messages/find_path/ResultRoutingTablesMessage.h"
#include "../io/storage/StorageHandler.h"
#include "lib/Path.h"
#include "lib/PathsCollection.h"
#include "../logger/Logger.h"

#include <vector>
#include <set>
#include <tuple>

class PathsManager {

public:

    PathsManager(
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLinesManager,
        StorageHandler* storageHandler,
        Logger *logger);

    void setContractorRoutingTables(ResultRoutingTablesMessage::Shared message);

    Path::Shared findPath();

    PathsCollection *pathCollection() const;

private:

    void findDirectPath();

    void findPathOnSecondLevel();

    void findPathOnThirdLevel();

    LoggerStream info() const;

    const string logHeader() const;

    // TODO : remove after testing
    void fillRoutingTables();

private:

    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    Logger *mLog;
    PathsCollection *mPathCollection;
    NodeUUID mNodeUUID;

    vector<pair<const NodeUUID, const TrustLineDirection>> contractorRT1;
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> contractorRT2;
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> contractorRT3;
    NodeUUID contractorUUID;

};


#endif //GEO_NETWORK_CLIENT_PATHSMANAGER_H

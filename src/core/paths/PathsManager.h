#ifndef GEO_NETWORK_CLIENT_PATHSMANAGER_H
#define GEO_NETWORK_CLIENT_PATHSMANAGER_H

#include "../trust_lines/manager/TrustLinesManager.h"
#include "../network/messages/find_path/ResultRoutingTablesMessage.h"
#include "../io/storage/StorageHandler.h"
#include "lib/Path.h"
#include "lib/PathsCollection.h"
#include "../logger/Logger.h"

#include <vector>
#include <unordered_map>

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

    void findPathsOnSecondLevel();

    void findPathsOnThirdLevel();

    void findPathsOnForthLevel();

    void findPathsOnFifthLevel();

    void findPathsOnSixthLevel();

    vector<NodeUUID> intermediateNodesOnContractorFirstLevel(
        const NodeUUID &thirdLevelSourceNode) const;

    LoggerStream info() const;

    const string logHeader() const;

    // TODO : remove after testing
    void fillRoutingTables();

    // TODO: remove after testing
    void testStorageHandler();

    // TODO: remove after testing
    void testStorageHandlerNextStep();

private:

    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    Logger *mLog;
    PathsCollection *mPathCollection;
    NodeUUID mNodeUUID;

    vector<NodeUUID> contractorRT1;
    unordered_map<NodeUUID, vector<NodeUUID>> contractorRT2;
    unordered_map<NodeUUID, vector<NodeUUID>> contractorRT3;
    NodeUUID contractorUUID;

};


#endif //GEO_NETWORK_CLIENT_PATHSMANAGER_H

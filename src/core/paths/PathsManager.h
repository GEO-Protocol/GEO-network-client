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
#include <set>

class PathsManager {

public:

    PathsManager(
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLinesManager,
        StorageHandler* storageHandler,
        Logger *logger);

    void setContractorRoutingTables(ResultRoutingTablesMessage::Shared message);

    Path::Shared findPath();

    // TODO : test
    Path::Shared findPathsTest();

    //PathsCollection *pathCollection() const;

    PathsCollection::Shared pathCollection() const;

private:

    void findDirectPath();

    void findPathsOnSecondLevel();

    void findPathsOnThirdLevel();

    void findPathsOnForthLevel();

    void findPathsOnFifthLevel();

    void findPathsOnSixthLevel();

    vector<NodeUUID> intermediateNodesOnContractorFirstLevel(
        const NodeUUID &thirdLevelSourceNode,
        const vector<NodeUUID> intermediateNodes) const;

    // test
    void findPathsOnSecondLevelTest();

    void findPathsOnThirdLevelTest();

    void findPathsOnForthLevelTest();

    void findPathsOnFifthLevelTest();

    void findPathsOnSixthLevelTest();

    vector<NodeUUID> intermediateNodesOnContractorFirstLevelTest(
            const NodeUUID &thirdLevelSourceNode) const;

    bool isPathValid(const Path &path);
    //test end

    LoggerStream info() const;

    const string logHeader() const;

    // TODO : remove after testing
    void fillRoutingTables();
    void fillBigRoutingTables();
    string nodeUUIDName(uint32_t number);
    NodeUUID* getPtrByNodeNumber(
        uint32_t number,
        vector<NodeUUID*> nodeUUIDPtrs);

    // TODO: remove after testing
    void testStorageHandler();
    void testTrustLineHandler();

private:

    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    Logger *mLog;
    //PathsCollection *mPathCollection;
    PathsCollection::Shared mPathCollection;
    NodeUUID mNodeUUID;

    vector<NodeUUID> contractorRT1;
    unordered_map<NodeUUID, vector<NodeUUID>> contractorRT2;
    unordered_map<NodeUUID, vector<NodeUUID>> contractorRT3;
    NodeUUID contractorUUID;

};


#endif //GEO_NETWORK_CLIENT_PATHSMANAGER_H

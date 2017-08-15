#ifndef GEO_NETWORK_CLIENT_PATHSMANAGER_H
#define GEO_NETWORK_CLIENT_PATHSMANAGER_H

#include "../trust_lines/manager/TrustLinesManager.h"
#include "../io/storage/StorageHandler.h"
#include "lib/Path.h"
#include "lib/PathsCollection.h"
#include "../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../logger/Logger.h"

#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <set>

class PathsManager {

public:
    PathsManager(
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLinesManager,
        StorageHandler* storageHandler,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        Logger &logger);

    void findPaths(
        const NodeUUID &contractorUUID,
        vector<NodeUUID> &contractorRT1,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT3);

    void findPathsOnSelfArea(
        const NodeUUID &contractorUUID);

    // TODO : test
    void findPathsTest(
        const NodeUUID &contractorUUID,
        vector<NodeUUID> &contractorRT1,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT3);

    ////// path by max flow
    TrustLineAmount buildPaths(
        const NodeUUID &contractorUUID);

    PathsCollection::Shared pathCollection() const;

    void clearPathsCollection();

private:
    void findDirectPath();

    void findPathsOnSecondLevel();

    void findPathsOnThirdLevel();

    void findPathsOnForthLevel(
        vector<NodeUUID> &contractorRT1);

    void findPathsOnFifthLevel(
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2);

    void findPathsOnSixthLevel(
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT3,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2);

    vector<NodeUUID> intermediateNodesOnContractorFirstLevel(
        const NodeUUID &thirdLevelSourceNode,
        vector<NodeUUID> &intermediateNodes,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2) const;

    // test
    void findPathsOnSecondLevelTest();

    void findPathsOnThirdLevelTest();

    void findPathsOnForthLevelTest(
        vector<NodeUUID> &contractorRT1);

    void findPathsOnFifthLevelTest(
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2);

    void findPathsOnSixthLevelTest(
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT3,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2);

    vector<NodeUUID> intermediateNodesOnContractorFirstLevelTest(
        const NodeUUID &thirdLevelSourceNode,
        unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2) const;

    bool isPathValid(const Path &path);
    //test end

    ////// path by max flow
    TrustLineAmount buildPathsOnOneLevel();

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    LoggerStream info() const;

    const string logHeader() const;

private:
    static const byte kMaxPathLength = 6;

private:
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    Logger &mLog;
    PathsCollection::Shared mPathCollection;
    NodeUUID mNodeUUID;
    NodeUUID mContractorUUID;

    vector<NodeUUID> passedNodeUUIDs;
    byte mCurrentPathLength;
};


#endif //GEO_NETWORK_CLIENT_PATHSMANAGER_H

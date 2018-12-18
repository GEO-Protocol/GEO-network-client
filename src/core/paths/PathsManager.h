#ifndef GEO_NETWORK_CLIENT_PATHSMANAGER_H
#define GEO_NETWORK_CLIENT_PATHSMANAGER_H

#include "lib/PathsCollection.h"
#include "../trust_lines/manager/TrustLinesManager.h"
#include "../topology/manager/TopologyTrustLinesManager.h"
#include "../logger/Logger.h"

#include <set>

class PathsManager {

public:
    PathsManager(
        const SerializedEquivalent equivalent,
        TrustLinesManager *trustLinesManager,
        TopologyTrustLinesManager *topologyTrustLineManager,
        Logger &logger);

    void buildPaths(
        BaseAddress::Shared contractorAddress,
        ContractorID contractorID);

    void addUsedAmount(
        BaseAddress::Shared sourceAddress,
        BaseAddress::Shared targetAddress,
        const TrustLineAmount &amount);

    void makeTrustLineFullyUsed(
        BaseAddress::Shared sourceAddress,
        BaseAddress::Shared targetAddress);

    // this method used for rebuild paths in case of insufficient founds
    void reBuildPaths(
        BaseAddress::Shared contractorAddress,
        const vector<BaseAddress::Shared> &inaccessibleNodes);

    PathsCollection::Shared pathCollection() const;

    void clearPathsCollection();

private:
    bool isPathValid(const Path &path);

    void buildPathsOnOneLevel();

    void buildPathsOnSecondLevel();

    TrustLineAmount calculateOneNode(
        ContractorID nodeID,
        const TrustLineAmount &currentFlow,
        byte level);

    TrustLineAmount reBuildPathsOnOneLevel();

    TrustLineAmount calculateOneNodeForRebuildingPaths(
        ContractorID nodeID,
        const TrustLineAmount& currentFlow,
        byte level);

    vector<BaseAddress::Shared> addressesPath();

    LoggerStream info() const;

    const string logHeader() const;

private:
    static const byte kMaxPathLength = 6;

private:
    TrustLinesManager *mTrustLinesManager;
    TopologyTrustLinesManager *mTopologyTrustLineManager;
    SerializedEquivalent mEquivalent;
    Logger &mLog;
    PathsCollection::Shared mPathCollection;
    BaseAddress::Shared mContractorAddress;
    ContractorID mContractorID;

    vector<ContractorID> mPassedNodeIDs;
    byte mCurrentPathLength;
    set<ContractorID> mInaccessibleNodes;
};


#endif //GEO_NETWORK_CLIENT_PATHSMANAGER_H

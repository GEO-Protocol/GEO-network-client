/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_PATHSMANAGER_H
#define GEO_NETWORK_CLIENT_PATHSMANAGER_H

#include "lib/Path.h"
#include "lib/PathsCollection.h"
#include "../trust_lines/manager/TrustLinesManager.h"
#include "../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../logger/Logger.h"

#include <set>

class PathsManager {

public:
    PathsManager(
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustLinesManager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        Logger &logger);

    void buildPaths(
        const NodeUUID &contractorUUID);

    void addUsedAmount(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID,
        const TrustLineAmount &amount);

    void makeTrustLineFullyUsed(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID);

    // this method used for rebuild paths in case of insufficient founds
    void reBuildPaths(
        const NodeUUID &contractorUUID,
        const set<NodeUUID> &inaccessibleNodes);

    PathsCollection::Shared pathCollection() const;

    void clearPathsCollection();

private:
    bool isPathValid(const Path &path);

    void buildPathsOnOneLevel();

    void buildPathsOnSecondLevel();

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    TrustLineAmount reBuildPathsOnOneLevel();

    TrustLineAmount calculateOneNodeForRebuildingPaths(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    LoggerStream info() const;

    const string logHeader() const;

private:
    static const byte kMaxPathLength = 6;

private:
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    Logger &mLog;
    PathsCollection::Shared mPathCollection;
    NodeUUID mNodeUUID;
    NodeUUID mContractorUUID;

    vector<NodeUUID> mPassedNodeUUIDs;
    byte mCurrentPathLength;
    set<NodeUUID> mInaccessibleNodes;
};


#endif //GEO_NETWORK_CLIENT_PATHSMANAGER_H

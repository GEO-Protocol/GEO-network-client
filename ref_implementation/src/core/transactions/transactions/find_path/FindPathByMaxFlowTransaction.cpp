/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "FindPathByMaxFlowTransaction.h"

FindPathByMaxFlowTransaction::FindPathByMaxFlowTransaction(
    NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const TransactionUUID &requestedTransactionUUID,
    PathsManager *pathsManager,
    ResourcesManager *resourcesManager,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationNodeCacheManager *MaxFlowCalculationNodeCacheManager,
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::FindPathByMaxFlowTransactionType,
        nodeUUID,
        manager,
        maxFlowCalculationTrustLineManager,
        maxFlowCalculationCacheManager,
        MaxFlowCalculationNodeCacheManager,
        logger),

    mContractorUUID(contractorUUID),
    mRequestedTransactionUUID(requestedTransactionUUID),
    mPathsManager(pathsManager),
    mResourcesManager(resourcesManager)
{}

TransactionResult::SharedConst FindPathByMaxFlowTransaction::sendRequestForCollectingTopology() {
    if (mContractorUUID == currentNodeUUID()) {
        warning() << "Attempt to initialise operation against itself was prevented. Canceled.";
        return resultDone();
    }
    debug() << "Build paths to " << mContractorUUID;
    try {
        vector<NodeUUID> contractors;
        contractors.push_back(mContractorUUID);
        const auto kTransaction = make_shared<CollectTopologyTransaction>(
            mNodeUUID,
            contractors,
            mTrustLinesManager,
            mMaxFlowCalculationTrustLineManager,
            mMaxFlowCalculationCacheManager,
            mMaxFlowCalculationNodeCacheManager,
            mLog);

        mMaxFlowCalculationTrustLineManager->setPreventDeleting(true);
        launchSubsidiaryTransaction(kTransaction);
    } catch (...) {
        warning() << "Can not launch Collecting Topology transaction for " << mContractorUUID << ".";
    }

    return resultAwakeAfterMilliseconds(
        kTopologyCollectingMillisecondsTimeout);
}

TransactionResult::SharedConst FindPathByMaxFlowTransaction::processCollectingTopology()
{
    fillTopology();
    mPathsManager->buildPaths(
        mContractorUUID);
    mResourcesManager->putResource(
        make_shared<PathsResource>(
            mRequestedTransactionUUID,
            mPathsManager->pathCollection()));
    mPathsManager->clearPathsCollection();
    mMaxFlowCalculationTrustLineManager->setPreventDeleting(false);
    return resultDone();
}

const string FindPathByMaxFlowTransaction::logHeader() const
{
    stringstream s;
    s << "[FindPathByMaxFlowTA: " << currentTransactionUUID() << "]";
    return s.str();
}

#include "FindPathByMaxFlowTransaction.h"

FindPathByMaxFlowTransaction::FindPathByMaxFlowTransaction(
    NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const TransactionUUID &requestedTransactionUUID,
    const SerializedEquivalent equivalent,
    PathsManager *pathsManager,
    ResourcesManager *resourcesManager,
    TrustLinesManager *manager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::FindPathByMaxFlowTransactionType,
        nodeUUID,
        equivalent,
        manager,
        topologyTrustLineManager,
        topologyCacheManager,
        maxFlowCacheManager,
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
            mEquivalent,
            contractors,
            mTrustLinesManager,
            mTopologyTrustLineManager,
            mTopologyCacheManager,
            mMaxFlowCacheManager,
            mLog);

        mTopologyTrustLineManager->setPreventDeleting(true);
        launchSubsidiaryTransaction(kTransaction);
    } catch (...) {
        warning() << "Can not launch Collecting Topology transaction for " << mContractorUUID << ".";
    }

    mCountProcessCollectingTopologyRun = 0;
    return resultAwakeAfterMilliseconds(
        kTopologyCollectingMillisecondsTimeout);
}

TransactionResult::SharedConst FindPathByMaxFlowTransaction::processCollectingTopology()
{
    auto const contextSize = mContext.size();
    fillTopology();
    mCountProcessCollectingTopologyRun++;
    if (contextSize > 0 && mCountProcessCollectingTopologyRun <= kCountRunningProcessCollectingTopologyStage) {
        return resultAwakeAfterMilliseconds(
            kTopologyCollectingAgainMillisecondsTimeout);
    }

    mPathsManager->buildPaths(
        mContractorUUID);
    mResourcesManager->putResource(
        make_shared<PathsResource>(
            mRequestedTransactionUUID,
            mPathsManager->pathCollection()));
    mPathsManager->clearPathsCollection();
    mTopologyTrustLineManager->setPreventDeleting(false);
    return resultDone();
}

const string FindPathByMaxFlowTransaction::logHeader() const
{
    stringstream s;
    s << "[FindPathByMaxFlowTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

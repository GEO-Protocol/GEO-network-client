#include "FindPathByMaxFlowTransaction.h"

FindPathByMaxFlowTransaction::FindPathByMaxFlowTransaction(
    NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    BaseAddress::Shared contractorAddress,
    const TransactionUUID &requestedTransactionUUID,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
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
        contractorsManager,
        manager,
        topologyTrustLineManager,
        topologyCacheManager,
        maxFlowCacheManager,
        logger),

    mContractorUUID(contractorUUID),
    mContractorAddress(contractorAddress),
    mRequestedTransactionUUID(requestedTransactionUUID),
    mPathsManager(pathsManager),
    mResourcesManager(resourcesManager)
{}

TransactionResult::SharedConst FindPathByMaxFlowTransaction::sendRequestForCollectingTopology()
{
    debug() << "Build paths to " << mContractorAddress->fullAddress();
    try {
        vector<BaseAddress::Shared> contractors;
        contractors.push_back(mContractorAddress);
        mContractorID = mTopologyTrustLineManager->getID(
            mContractorAddress);
        info() << "ContractorID " << mContractorID;
        const auto kTransaction = make_shared<CollectTopologyTransaction>(
            mNodeUUID,
            mEquivalent,
            contractors,
            mContractorsManager,
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
            mContractorAddress,
            mContractorID);

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

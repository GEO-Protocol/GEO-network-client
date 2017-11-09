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
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::FindPathByMaxFlowTransactionType,
        nodeUUID,
        manager,
        maxFlowCalculationTrustLineManager,
        maxFlowCalculationCacheManager,
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
            mLog);

        mMaxFlowCalculationTrustLineManager->setPreventDeleting(true);
        launchSubsidiaryTransaction(kTransaction);
    } catch (...) {
        warning() << "Can not launch Collecting Topology transaction for " << mContractorUUID << ".";
    }

    return resultAwaikAfterMilliseconds(
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

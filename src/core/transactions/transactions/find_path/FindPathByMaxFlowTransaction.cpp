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

    BaseTransaction(
        BaseTransaction::TransactionType::FindPathByMaxFlowTransactionType,
        nodeUUID,
        logger),

    mContractorUUID(contractorUUID),
    mRequestedTransactionUUID(requestedTransactionUUID),
    mPathsManager(pathsManager),
    mResourcesManager(resourcesManager),
    mTrustLineManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager)
{}

TransactionResult::SharedConst FindPathByMaxFlowTransaction::run()
{
    switch (mStep) {
        case Stages::SendRequestForGettingRoutingTables:
            if (mContractorUUID == currentNodeUUID()) {
                error() << "Attempt to initialise operation against itself was prevented. Canceled.";
                return resultDone();
            }
            mStep = Stages::BuildAllPaths;
            debug() << "Build paths to " << mContractorUUID;
            try {
                vector<NodeUUID> contractors;
                contractors.push_back(mContractorUUID);
                const auto kTransaction = make_shared<CollectTopologyTransaction>(
                    mNodeUUID,
                    contractors,
                    mTrustLineManager,
                    mMaxFlowCalculationTrustLineManager,
                    mMaxFlowCalculationCacheManager,
                    mLog);

                mMaxFlowCalculationTrustLineManager->setPreventDeleting(true);
                launchSubsidiaryTransaction(kTransaction);
            } catch (...) {
                error() << "Can not launch Collecting Topology transaction for " << mContractorUUID << ".";
            }

            return resultAwaikAfterMilliseconds(
                kTopologyCollectingMilisecondsTimeout);

        case Stages::BuildAllPaths:
            mPathsManager->buildPaths(
                mContractorUUID);
            mResourcesManager->putResource(
                make_shared<PathsResource>(
                    mRequestedTransactionUUID,
                    mPathsManager->pathCollection()));
            mMaxFlowCalculationTrustLineManager->setPreventDeleting(false);
            mStep = Stages::SendRequestForGettingRoutingTables;
            return resultDone();
        default:
            throw ValueError("FindPathByMaxFlowTransaction::run: "
                                     "wrong value of mStep");
    }
}

const string FindPathByMaxFlowTransaction::logHeader() const
{
    stringstream s;
    s << "[FindPathByMaxFlowTA: " << currentTransactionUUID() << "]";
    return s.str();
}

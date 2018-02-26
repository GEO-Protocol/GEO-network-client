#include "CollectTopologyTransaction.h"

CollectTopologyTransaction::CollectTopologyTransaction(
    const NodeUUID &nodeUUID,
    const vector<NodeUUID> &contractors,
    TrustLinesManager *manager,
    TopologyTrustLineManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::CollectTopologyTransactionType,
        nodeUUID,
        logger),
    mContractors(contractors),
    mTrustLinesManager(manager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{}

TransactionResult::SharedConst CollectTopologyTransaction::run()
{
    debug() << "Collect topology to " << mContractors.size() << " contractors";
    // Check if Node does not have outgoing FlowAmount;
    if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().empty()){
        return resultDone();
    }
    sendMessagesToContractors();
    if (!mTopologyCacheManager->isInitiatorCached()) {
        for (auto const &nodeUUIDAndOutgoingFlow : mTrustLinesManager->outgoingFlows()) {
            auto trustLineAmountShared = nodeUUIDAndOutgoingFlow.second;
            mTopologyTrustLineManager->addTrustLine(
                make_shared<TopologyTrustLine>(
                    mNodeUUID,
                    nodeUUIDAndOutgoingFlow.first,
                    trustLineAmountShared));
        }
        sendMessagesOnFirstLevel();
        mTopologyCacheManager->setInitiatorCache();
    }
    return resultDone();
}

void CollectTopologyTransaction::sendMessagesToContractors()
{
    for (const auto &contractorUUID : mContractors)
        sendMessage<InitiateMaxFlowCalculationMessage>(
            contractorUUID,
            currentNodeUUID());
}

void CollectTopologyTransaction::sendMessagesOnFirstLevel()
{
    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
        sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
            nodeUUIDOutgoingFlow,
            mNodeUUID);
    }
}

const string CollectTopologyTransaction::logHeader() const
{
    stringstream s;
    s << "[CollectTopologyTA: " << currentTransactionUUID() << "]";
    return s.str();
}

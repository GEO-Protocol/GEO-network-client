#include "CollectTopologyTransaction.h"

CollectTopologyTransaction::CollectTopologyTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    const vector<NodeUUID> &contractors,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::CollectTopologyTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractors(contractors),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{}

CollectTopologyTransaction::CollectTopologyTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    const vector<BaseAddress::Shared> &contractorAddresses,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::CollectTopologyTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorAddresses(contractorAddresses),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{}

TransactionResult::SharedConst CollectTopologyTransaction::run()
{
    debug() << "Collect topology to " << mContractorAddresses.size() << " contractors";
    // Check if Node does not have outgoing FlowAmount;
    if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().empty()){
        return resultDone();
    }
    sendMessagesToContractors();
    for (auto const &nodeUUIDAndOutgoingFlow : mTrustLinesManager->outgoingFlows()) {
        auto trustLineAmountShared = nodeUUIDAndOutgoingFlow.second;
        mTopologyTrustLineManager->addTrustLine(
            make_shared<TopologyTrustLine>(
                mNodeUUID,
                nodeUUIDAndOutgoingFlow.first,
                trustLineAmountShared));
    }
    for (auto const &nodeAddressAndOutgoingFlow : mTrustLinesManager->outgoingFlowsNew()) {
        auto targetID = mTopologyTrustLineManager->getID(nodeAddressAndOutgoingFlow.first);
        auto trustLineAmountShared = nodeAddressAndOutgoingFlow.second;
        mTopologyTrustLineManager->addTrustLineNew(
            make_shared<TopologyTrustLineNew>(
                0,
                targetID,
                trustLineAmountShared));
    }
    if (!mTopologyCacheManager->isInitiatorCached()) {
        sendMessagesOnFirstLevel();
        mTopologyCacheManager->setInitiatorCache();
    }
    return resultDone();
}

void CollectTopologyTransaction::sendMessagesToContractors()
{
    for (const auto &contractorAddress : mContractorAddresses)
        sendMessage<InitiateMaxFlowCalculationMessage>(
            contractorAddress,
            mEquivalent,
            currentNodeUUID(),
            mContractorsManager->ownAddresses());
}

void CollectTopologyTransaction::sendMessagesOnFirstLevel()
{
    auto outgoingFlowIDs = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    auto outgoingFlowIDIt = outgoingFlowIDs.begin();
    while (outgoingFlowIDIt != outgoingFlowIDs.end()) {
        // firstly send message to gateways
        if (mTrustLinesManager->isContractorGateway(*outgoingFlowIDIt)) {
            sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
                *outgoingFlowIDIt,
                mEquivalent,
                mNodeUUID,
                mContractorsManager->idOnContractorSide(*outgoingFlowIDIt));
            outgoingFlowIDs.erase(outgoingFlowIDIt);
        } else {
            outgoingFlowIDIt++;
        }
    }
    for (auto const &nodeIDWithOutgoingFlow : outgoingFlowIDs) {
        sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
            nodeIDWithOutgoingFlow,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->idOnContractorSide(nodeIDWithOutgoingFlow));
    }
}

const string CollectTopologyTransaction::logHeader() const
{
    stringstream s;
    s << "[CollectTopologyTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

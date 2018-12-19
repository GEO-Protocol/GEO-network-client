#include "CollectTopologyTransaction.h"

CollectTopologyTransaction::CollectTopologyTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    const vector<BaseAddress::Shared> &contractorAddresses,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    bool iAmGateway,
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
    mMaxFlowCacheManager(maxFlowCacheManager),
    mIamGateway(iAmGateway)
{}

TransactionResult::SharedConst CollectTopologyTransaction::run()
{
    debug() << "Collect topology to " << mContractorAddresses.size() << " contractors";
    // Check if Node does not have outgoing FlowAmount;
    if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().empty()){
        return resultDone();
    }
    sendMessagesToContractors();
    if (mIamGateway) {
        for (auto const &nodeUUIDAndOutgoingFlow : mTrustLinesManager->outgoingFlows()) {
            if (mTrustLinesManager->isContractorGateway(nodeUUIDAndOutgoingFlow.first)) {
                auto trustLineAmountShared = nodeUUIDAndOutgoingFlow.second;
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        mNodeUUID,
                        nodeUUIDAndOutgoingFlow.first,
                        trustLineAmountShared));
                continue;
            }
            if (find(mContractors.begin(), mContractors.end(), nodeUUIDAndOutgoingFlow.first) != mContractors.end()) {
                auto trustLineAmountShared = nodeUUIDAndOutgoingFlow.second;
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        mNodeUUID,
                        nodeUUIDAndOutgoingFlow.first,
                        trustLineAmountShared));
            }
        }
    } else {
        for (auto const &nodeUUIDAndOutgoingFlow : mTrustLinesManager->outgoingFlows()) {
            auto trustLineAmountShared = nodeUUIDAndOutgoingFlow.second;
            mTopologyTrustLineManager->addTrustLine(
                make_shared<TopologyTrustLine>(
                    mNodeUUID,
                    nodeUUIDAndOutgoingFlow.first,
                    trustLineAmountShared));
        }
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
            mIamGateway);
}

void CollectTopologyTransaction::sendMessagesOnFirstLevel()
{
    if (mIamGateway) {
        vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelGatewayNeighborsWithOutgoingFlow();
        for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
            if (find(mContractors.begin(), mContractors.end(), nodeUUIDOutgoingFlow) != mContractors.end()) {
                continue;
            }
            sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
                nodeUUIDOutgoingFlow,
                mEquivalent,
                mNodeUUID);
        }
    } else {
        vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
        auto outgoingFlowUuidIt = outgoingFlowUuids.begin();
        while (outgoingFlowUuidIt != outgoingFlowUuids.end()) {
            if (find(mContractors.begin(), mContractors.end(), *outgoingFlowUuidIt) != mContractors.end()) {
                outgoingFlowUuids.erase(outgoingFlowUuidIt);
                continue;
            }
            // firstly send message to gateways
            if (mTrustLinesManager->isContractorGateway(*outgoingFlowUuidIt)) {
                sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
                    *outgoingFlowUuidIt,
                    mEquivalent,
                    mNodeUUID);
                outgoingFlowUuids.erase(outgoingFlowUuidIt);
            } else {
                outgoingFlowUuidIt++;
            }
        }
        for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
            sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
                nodeUUIDOutgoingFlow,
                mEquivalent,
                mNodeUUID);
        }
    }
}

const string CollectTopologyTransaction::logHeader() const
{
    stringstream s;
    s << "[CollectTopologyTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

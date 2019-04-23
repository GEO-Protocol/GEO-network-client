#include "CollectTopologyTransaction.h"

CollectTopologyTransaction::CollectTopologyTransaction(
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
        BaseTransaction::CollectTopologyTransactionType,
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
    if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().first.empty()){
        return resultDone();
    }
    sendMessagesToContractors();

    if (mIamGateway) {
        for (auto const &nodeAddressAndOutgoingFlow : mTrustLinesManager->outgoingFlows()) {
            auto targetID = mTopologyTrustLineManager->getID(nodeAddressAndOutgoingFlow.first);
            auto trustLineAmountShared = nodeAddressAndOutgoingFlow.second;
            if (mTrustLinesManager->isContractorGateway(targetID)) {
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        +TopologyTrustLinesManager::kCurrentNodeID,
                        targetID,
                        trustLineAmountShared));
                continue;
            }
            if (isNodeListedInTransactionContractors(nodeAddressAndOutgoingFlow.first)) {
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        +TopologyTrustLinesManager::kCurrentNodeID,
                        targetID,
                        trustLineAmountShared));
            }
        }
    } else {
        for (auto const &nodeAddressAndOutgoingFlow : mTrustLinesManager->outgoingFlows()) {
            auto targetID = mTopologyTrustLineManager->getID(nodeAddressAndOutgoingFlow.first);
            auto trustLineAmountShared = nodeAddressAndOutgoingFlow.second;
            mTopologyTrustLineManager->addTrustLine(
                make_shared<TopologyTrustLine>(
                    +TopologyTrustLinesManager::kCurrentNodeID,
                    targetID,
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
            mContractorsManager->ownAddresses(),
            mIamGateway);
}

void CollectTopologyTransaction::sendMessagesOnFirstLevel()
{
    if (mIamGateway) {
        auto outgoingFlowIDs = mTrustLinesManager->firstLevelGatewayNeighborsWithOutgoingFlow().first;
        for (auto const &nodeIDOutgoingFlow : outgoingFlowIDs) {
            auto contractorAddress = mContractorsManager->contractorMainAddress(nodeIDOutgoingFlow);
            if (isNodeListedInTransactionContractors(contractorAddress)) {
                continue;
            }
            sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
                nodeIDOutgoingFlow,
                mEquivalent,
                mContractorsManager->idOnContractorSide(nodeIDOutgoingFlow));
        }
    } else {
        auto outgoingFlowIDs = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().first;
        auto outgoingFlowIDIt = outgoingFlowIDs.begin();
        while (outgoingFlowIDIt != outgoingFlowIDs.end()) {
            auto contractorAddress = mContractorsManager->contractorMainAddress(*outgoingFlowIDIt);
            if (isNodeListedInTransactionContractors(contractorAddress)) {
                outgoingFlowIDIt++;
                continue;
            }
            // firstly send message to gateways
            if (mTrustLinesManager->isContractorGateway(*outgoingFlowIDIt)) {
                sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
                    *outgoingFlowIDIt,
                    mEquivalent,
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
                mContractorsManager->idOnContractorSide(nodeIDWithOutgoingFlow));
        }
    }
}

bool CollectTopologyTransaction::isNodeListedInTransactionContractors(
    BaseAddress::Shared nodeAddress) const
{
    for (const auto &contractor : mContractorAddresses) {
        if (nodeAddress == contractor) {
            return true;
        }
    }
    return false;
}

const string CollectTopologyTransaction::logHeader() const
{
    stringstream s;
    s << "[CollectTopologyTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

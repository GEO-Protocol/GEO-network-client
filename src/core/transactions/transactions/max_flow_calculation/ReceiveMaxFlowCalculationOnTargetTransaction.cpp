#include "ReceiveMaxFlowCalculationOnTargetTransaction.h"

ReceiveMaxFlowCalculationOnTargetTransaction::ReceiveMaxFlowCalculationOnTargetTransaction(
    const NodeUUID &nodeUUID,
    InitiateMaxFlowCalculationMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveMaxFlowCalculationOnTargetTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mTopologyCacheManager(topologyCacheManager)
{}

TransactionResult::SharedConst ReceiveMaxFlowCalculationOnTargetTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "target: " << mNodeUUID;
    info() << "run\t" << "initiator: " << mMessage->senderUUID;
#endif
    sendResultToInitiator();
    sendMessagesOnFirstLevel();
    return resultDone();
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendResultToInitiator()
{
    TopologyCache::Shared maxFlowCalculationCachePtr
        = mTopologyCacheManager->cacheByAddress(mMessage->senderAddresses.at(0));
    TopologyCacheNew::Shared maxFlowCalculationCachePtrNew
        = mTopologyCacheManager->cacheByAddressNew(mMessage->senderAddresses.at(0));
    if (maxFlowCalculationCachePtr != nullptr and maxFlowCalculationCachePtrNew != nullptr) {
        sendCachedResultToInitiator(
            maxFlowCalculationCachePtr,
            maxFlowCalculationCachePtrNew);
        return;
    }
    if (maxFlowCalculationCachePtr != nullptr or maxFlowCalculationCachePtrNew != nullptr) {
        warning() << "Problem with cache!!!";
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount() && incomingFlow.first != mMessage->senderUUID) {
            incomingFlows.push_back(
                incomingFlow);
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsNew;
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsNew;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsNew()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount() && incomingFlow.first != mMessage->senderAddresses.at(0)) {
            incomingFlowsNew.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResultToInitiator\t" << "send to " << mMessage->senderUUID;
    info() << "sendResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();
#endif
    if (!incomingFlows.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderAddresses.at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            outgoingFlows,
            incomingFlows,
            outgoingFlowsNew,
            incomingFlowsNew);
        mTopologyCacheManager->addCache(
            mMessage->senderAddresses.at(0),
            make_shared<TopologyCache>(
                outgoingFlows,
                incomingFlows));
        mTopologyCacheManager->addCacheNew(
            mMessage->senderAddresses.at(0),
            make_shared<TopologyCacheNew>(
                outgoingFlowsNew,
                incomingFlowsNew));
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr,
    TopologyCacheNew::Shared maxFlowCalculationCachePtrNew)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->senderUUID
           << " " << mMessage->senderAddresses.at(0)->fullAddress();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(
                incomingFlow);
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsForSendingNew;
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsForSendingNew;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsNew()) {
        if (incomingFlow.first != mMessage->senderAddresses.at(0)
            && !maxFlowCalculationCachePtrNew->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSendingNew.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size()
           << " IncomingFlowsNew: " << incomingFlowsForSendingNew.size();
#endif
    if (!incomingFlowsForSending.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderAddresses.at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            outgoingFlowsForSending,
            incomingFlowsForSending,
            outgoingFlowsForSendingNew,
            incomingFlowsForSendingNew);
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendMessagesOnFirstLevel()
{
    auto incomingFlowIDs = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    auto initiatorContractorID = mContractorsManager->contractorIDByAddress(mMessage->senderAddresses.at(0));
    for (auto const &nodeIDWithIncomingFlow : incomingFlowIDs) {
        if (nodeIDWithIncomingFlow == initiatorContractorID) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeIDWithIncomingFlow;
#endif
        sendMessage<MaxFlowCalculationTargetFstLevelMessage>(
            nodeIDWithIncomingFlow,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->idOnContractorSide(nodeIDWithIncomingFlow),
            mMessage->senderUUID,
            mMessage->senderAddresses);
    }
}

const string ReceiveMaxFlowCalculationOnTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveMaxFlowCalculationOnTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

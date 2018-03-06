#include "ReceiveMaxFlowCalculationOnTargetTransaction.h"

ReceiveMaxFlowCalculationOnTargetTransaction::ReceiveMaxFlowCalculationOnTargetTransaction(
    const NodeUUID &nodeUUID,
    InitiateMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveMaxFlowCalculationOnTargetTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLinesManager(trustLinesManager),
    mTopologyCacheManager(topologyCacheManager)
{}

InitiateMaxFlowCalculationMessage::Shared ReceiveMaxFlowCalculationOnTargetTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst ReceiveMaxFlowCalculationOnTargetTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "target: " << mNodeUUID;
    info() << "run\t" << "initiator: " << mMessage->senderUUID;
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();
#endif
    sendResultToInitiator();
    sendMessagesOnFirstLevel();
    return resultDone();
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendResultToInitiator()
{
    TopologyCache::Shared maxFlowCalculationCachePtr
        = mTopologyCacheManager->cacheByNode(mMessage->senderUUID);
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount() && incomingFlow.first != mMessage->senderUUID) {
            incomingFlows.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResultToInitiator\t" << "send to " << mMessage->senderUUID;
    info() << "sendResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();
#endif
    if (!incomingFlows.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            outgoingFlows,
            incomingFlows);
        mTopologyCacheManager->addCache(
            mMessage->senderUUID,
            make_shared<TopologyCache>(
                outgoingFlows,
                incomingFlows));
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->senderUUID;
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
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();
#endif
    if (!incomingFlowsForSending.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendMessagesOnFirstLevel()
{
    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        if (nodeUUIDIncomingFlow == mMessage->senderUUID) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeUUIDIncomingFlow;
#endif
        sendMessage<MaxFlowCalculationTargetFstLevelMessage>(
            nodeUUIDIncomingFlow,
            mEquivalent,
            mNodeUUID,
            mMessage->senderUUID);
    }
}

const string ReceiveMaxFlowCalculationOnTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveMaxFlowCalculationOnTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

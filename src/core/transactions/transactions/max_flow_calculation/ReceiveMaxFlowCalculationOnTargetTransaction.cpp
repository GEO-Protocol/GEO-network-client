#include "ReceiveMaxFlowCalculationOnTargetTransaction.h"

ReceiveMaxFlowCalculationOnTargetTransaction::ReceiveMaxFlowCalculationOnTargetTransaction(
    const NodeUUID &nodeUUID,
    InitiateMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveMaxFlowCalculationOnTargetTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager)
{}

InitiateMaxFlowCalculationMessage::Shared ReceiveMaxFlowCalculationOnTargetTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst ReceiveMaxFlowCalculationOnTargetTransaction::run()
{
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
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
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr
        = mMaxFlowCalculationCacheManager->cacheByNode(mMessage->senderUUID);
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
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    info() << "sendResultToInitiator\t" << "send to " << mMessage->senderUUID;
    info() << "sendResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();
#endif
    if (incomingFlows.size() > 0) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderUUID,
            mNodeUUID,
            outgoingFlows,
            incomingFlows);
        mMaxFlowCalculationCacheManager->addCache(
            mMessage->senderUUID,
            make_shared<MaxFlowCalculationCache>(
                outgoingFlows,
                incomingFlows));
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr)
{
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
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
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();
#endif
    if (incomingFlowsForSending.size() > 0) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderUUID,
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
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
        info() << "sendFirst\t" << nodeUUIDIncomingFlow;
#endif
        sendMessage<MaxFlowCalculationTargetFstLevelMessage>(
            nodeUUIDIncomingFlow,
            mNodeUUID,
            mMessage->senderUUID);
    }
}

const string ReceiveMaxFlowCalculationOnTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveMaxFlowCalculationOnTargetTA: " << currentTransactionUUID() << "]";
    return s.str();
}

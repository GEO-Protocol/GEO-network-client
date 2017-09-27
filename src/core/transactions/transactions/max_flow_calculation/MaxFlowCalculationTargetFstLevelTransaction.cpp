#include "MaxFlowCalculationTargetFstLevelTransaction.h"

MaxFlowCalculationTargetFstLevelTransaction::MaxFlowCalculationTargetFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetFstLevelMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetFstLevelTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mIAmGateway(iAmGateway)
{}

MaxFlowCalculationTargetFstLevelMessage::Shared MaxFlowCalculationTargetFstLevelTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationTargetFstLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "target: " << mMessage->targetUUID();
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();
#endif
    vector<NodeUUID> incomingFlowUuids;
    if (mIAmGateway) {
        sendResultToInitiator();
        incomingFlowUuids = mTrustLinesManager->firstLevelGatewayNeighborsWithIncomingFlow();
    } else {
        incomingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    }
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        if (nodeUUIDIncomingFlow == mMessage->senderUUID || nodeUUIDIncomingFlow == mMessage->targetUUID()) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeUUIDIncomingFlow;
#endif
        sendMessage<MaxFlowCalculationTargetSndLevelMessage>(
            nodeUUIDIncomingFlow,
            mNodeUUID,
            mMessage->targetUUID());
    }
    return resultDone();
}

void MaxFlowCalculationTargetFstLevelTransaction::sendResultToInitiator()
{
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr
            = mMaxFlowCalculationCacheManager->cacheByNode(mMessage->targetUUID());
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount()
            && incomingFlow.first != mMessage->senderUUID
            && incomingFlow.first != mMessage->targetUUID()) {
            incomingFlows.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID();
    info() << "sendResultToInitiator\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();
#endif
    if (incomingFlows.size() > 0) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetUUID(),
            mNodeUUID,
            outgoingFlows,
            incomingFlows);
        mMaxFlowCalculationCacheManager->addCache(
            mMessage->targetUUID(),
            make_shared<MaxFlowCalculationCache>(
                outgoingFlows,
                incomingFlows));
    }
}

void MaxFlowCalculationTargetFstLevelTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID
            && incomingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size();
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();
#endif
    if (incomingFlowsForSending.size() > 0) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetUUID(),
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

const string MaxFlowCalculationTargetFstLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTargetFstLevelTA: " << currentTransactionUUID() << "]";
    return s.str();
}

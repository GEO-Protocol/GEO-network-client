#include "MaxFlowCalculationSourceFstLevelTransaction.h"

MaxFlowCalculationSourceFstLevelTransaction::MaxFlowCalculationSourceFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceFstLevelMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceFstLevelTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(trustLinesManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mIAmGateway(iAmGateway)
{}

MaxFlowCalculationSourceFstLevelMessage::Shared MaxFlowCalculationSourceFstLevelTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationSourceFstLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();
#endif
    vector<NodeUUID> outgoingFlowUuids;
    if (mIAmGateway) {
        sendResultToInitiator();
        outgoingFlowUuids = mTrustLinesManager->firstLevelGatewayNeighborsWithOutgoingFlow();
    } else {
        outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    }
    for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
        if (nodeUUIDOutgoingFlow == mMessage->senderUUID) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeUUIDOutgoingFlow;
#endif
        sendMessage<MaxFlowCalculationSourceSndLevelMessage>(
            nodeUUIDOutgoingFlow,
            mNodeUUID,
            mMessage->senderUUID);
    }
    return resultDone();
}

void MaxFlowCalculationSourceFstLevelTransaction::sendResultToInitiator()
{
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr
            = mMaxFlowCalculationCacheManager->cacheByNode(mMessage->senderUUID);
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()
            && outgoingFlow.first != mMessage->senderUUID) {
            outgoingFlows.push_back(
                outgoingFlow);
        }
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResult\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResult\t" << "IncomingFlows: " << incomingFlows.size();
#endif
    if (outgoingFlows.size() > 0) {
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

void MaxFlowCalculationSourceFstLevelTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID
            && !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
            outgoingFlowsForSending.push_back(
               outgoingFlow);
        }
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size();
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();
#endif
    if (outgoingFlowsForSending.size() > 0) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderUUID,
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

const string MaxFlowCalculationSourceFstLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationSourceFstLevelTA: " << currentTransactionUUID() << "]";
    return s.str();
}

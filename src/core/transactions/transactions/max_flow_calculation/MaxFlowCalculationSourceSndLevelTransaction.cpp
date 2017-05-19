#include "MaxFlowCalculationSourceSndLevelTransaction.h"

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceSndLevelMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceSndLevelTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager)
{}

MaxFlowCalculationSourceSndLevelMessage::Shared MaxFlowCalculationSourceSndLevelTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationSourceSndLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "target: " << mMessage->targetUUID();
#endif
    sendResultToInitiator();
    return resultDone();
}

void MaxFlowCalculationSourceSndLevelTransaction::sendResultToInitiator()
{
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr
        = mMaxFlowCalculationCacheManager->cacheByNode(mMessage->targetUUID());
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
            && outgoingFlow.first != mMessage->senderUUID
            && outgoingFlow.first != mMessage->targetUUID()) {
            outgoingFlows.push_back(
                outgoingFlow);
        }
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount()
            && incomingFlow.first == mMessage->senderUUID) {
            incomingFlows.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResult\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResult\t" << "IncomingFlows: " << incomingFlows.size();
#endif
    if (outgoingFlows.size() > 0 || incomingFlows.size() > 0) {
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

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID
            && outgoingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
            outgoingFlowsForSending.push_back(
                outgoingFlow);
        }
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        auto trustLineAmountShared = incomingFlow.second;
        if (incomingFlow.first == mMessage->senderUUID
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size();
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();
#endif
    if (outgoingFlowsForSending.size() > 0 || incomingFlowsForSending.size() > 0) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetUUID(),
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

const string MaxFlowCalculationSourceSndLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationSourceSndLevelTA: " << currentTransactionUUID() << "]";
    return s.str();
}
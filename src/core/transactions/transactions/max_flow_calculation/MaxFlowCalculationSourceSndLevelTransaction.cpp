#include "MaxFlowCalculationSourceSndLevelTransaction.h"

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceSndLevelMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceSndLevelTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mIAmGateway(iAmGateway)
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
    if (mIAmGateway) {
        sendGatewayResultToInitiator();
    } else {
        sendResultToInitiator();
    }
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
    const auto incomingFlow = mTrustLinesManager->incomingFlow(mMessage->senderUUID);
    if (*incomingFlow.second.get() > TrustLine::kZeroAmount()) {
        incomingFlows.push_back(
            incomingFlow);
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
    auto const incomingFlow = mTrustLinesManager->incomingFlow(mMessage->senderUUID);
    if (!maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
        incomingFlowsForSending.push_back(
            incomingFlow);
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

void MaxFlowCalculationSourceSndLevelTransaction::sendGatewayResultToInitiator()
{
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr
            = mMaxFlowCalculationCacheManager->cacheByNode(mMessage->targetUUID());
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedGatewayResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlowsToGateways()) {
        if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()
            && outgoingFlow.first != mMessage->senderUUID
            && outgoingFlow.first != mMessage->targetUUID()) {
            outgoingFlows.push_back(
                outgoingFlow);
        }
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    const auto incomingFlow = mTrustLinesManager->incomingFlow(mMessage->senderUUID);
    if (*incomingFlow.second.get() > TrustLine::kZeroAmount()) {
        incomingFlows.push_back(
            incomingFlow);
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResult\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResult\t" << "IncomingFlows: " << incomingFlows.size();
#endif
    if (outgoingFlows.size() > 0 || incomingFlows.size() > 0) {
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
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

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedGatewayResultToInitiator(
        MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlowsToGateways()) {
        if (outgoingFlow.first != mMessage->senderUUID
            && outgoingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
            outgoingFlowsForSending.push_back(
                outgoingFlow);
        }
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    auto const incomingFlow = mTrustLinesManager->incomingFlow(mMessage->senderUUID);
    if (!maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
        incomingFlowsForSending.push_back(
            incomingFlow);
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size();
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();
#endif
    if (outgoingFlowsForSending.size() > 0 || incomingFlowsForSending.size() > 0) {
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
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
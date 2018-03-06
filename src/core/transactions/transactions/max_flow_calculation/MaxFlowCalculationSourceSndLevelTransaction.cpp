#include "MaxFlowCalculationSourceSndLevelTransaction.h"

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceSndLevelMessage::Shared message,
    TrustLinesManager *manager,
    TopologyCacheManager *topologyCacheManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceSndLevelTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mTopologyCacheManager(topologyCacheManager),
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
    info() << "run\t" << "i am is gateway: " << mIAmGateway;
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
    TopologyCache::Shared maxFlowCalculationCachePtr
        = mTopologyCacheManager->cacheByNode(mMessage->targetUUID());
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
    if (!outgoingFlows.empty() || !incomingFlows.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetUUID(),
            mEquivalent,
            mNodeUUID,
            outgoingFlows,
            incomingFlows);
        mTopologyCacheManager->addCache(
            mMessage->targetUUID(),
            make_shared<TopologyCache>(
                outgoingFlows,
                incomingFlows));
    }
}

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr)
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
    if (!outgoingFlowsForSending.empty() || !incomingFlowsForSending.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetUUID(),
            mEquivalent,
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

void MaxFlowCalculationSourceSndLevelTransaction::sendGatewayResultToInitiator()
{
    TopologyCache::Shared maxFlowCalculationCachePtr
            = mTopologyCacheManager->cacheByNode(mMessage->targetUUID());
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedGatewayResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendGatewayResultToInitiator\t" << "send to " << mMessage->targetUUID();
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
    info() << "sendGatewayResult\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendGatewayResult\t" << "IncomingFlows: " << incomingFlows.size();
#endif
    if (!outgoingFlows.empty() || !incomingFlows.empty()) {
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
            mMessage->targetUUID(),
            mEquivalent,
            mNodeUUID,
            outgoingFlows,
            incomingFlows);
        mTopologyCacheManager->addCache(
            mMessage->targetUUID(),
            make_shared<TopologyCache>(
                outgoingFlows,
                incomingFlows));
    }
}

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedGatewayResultToInitiator(
        TopologyCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedGatewayResultToInitiator\t" << "send to " << mMessage->targetUUID();
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
    info() << "sendCachedGatewayResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size();
    info() << "sendCachedGatewayResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();
#endif
    if (!outgoingFlowsForSending.empty() || !incomingFlowsForSending.empty()) {
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
            mMessage->targetUUID(),
            mEquivalent,
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

const string MaxFlowCalculationSourceSndLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationSourceSndLevelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
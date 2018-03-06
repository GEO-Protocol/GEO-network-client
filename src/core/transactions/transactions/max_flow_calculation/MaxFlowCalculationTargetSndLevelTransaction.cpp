#include "MaxFlowCalculationTargetSndLevelTransaction.h"

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetSndLevelMessage::Shared message,
    TrustLinesManager *manager,
    TopologyCacheManager *topologyCacheManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetSndLevelTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mTopologyCacheManager(topologyCacheManager),
    mIAmGateway(iAmGateway)
{}

MaxFlowCalculationTargetSndLevelMessage::Shared MaxFlowCalculationTargetSndLevelTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationTargetSndLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID.stringUUID();
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "target: " << mMessage->targetUUID();
    info() << "run\t" << "i am is gateway: " << mIAmGateway;
#endif
    if (mIAmGateway) {
        sendGatewayResultToInitiator();
    } else {
        sendResultToInitiator();
    }
    return resultDone();
}

void MaxFlowCalculationTargetSndLevelTransaction::sendResultToInitiator()
{
    TopologyCache::Shared maxFlowCalculationCachePtr
        = mTopologyCacheManager->cacheByNode(mMessage->targetUUID());
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    auto const outgoingFlow = mTrustLinesManager->outgoingFlow(
        mMessage->senderUUID);
    if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()) {
        outgoingFlows.push_back(
            outgoingFlow);
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromNonGateways()) {
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

void MaxFlowCalculationTargetSndLevelTransaction::sendCachedResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    auto const outgoingFlow = mTrustLinesManager->outgoingFlow(
        mMessage->senderUUID);
    if (!maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
        outgoingFlowsForSending.push_back(
            outgoingFlow);
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromNonGateways()) {
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
    if (!outgoingFlowsForSending.empty() || !incomingFlowsForSending.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetUUID(),
            mEquivalent,
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

void MaxFlowCalculationTargetSndLevelTransaction::sendGatewayResultToInitiator()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    TopologyCache::Shared maxFlowCalculationCachePtr
            = mTopologyCacheManager->cacheByNode(mMessage->targetUUID());
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedGatewayResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    auto const outgoingFlow = mTrustLinesManager->outgoingFlow(
        mMessage->senderUUID);
    if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()) {
        outgoingFlows.push_back(
            outgoingFlow);
    }

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
    info() << "sendGatewayResultToInitiator\t" << "send to " << mMessage->targetUUID();
    info() << "sendGatewayResultToInitiator\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendGatewayResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();
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

void MaxFlowCalculationTargetSndLevelTransaction::sendCachedGatewayResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedGatewayResultToInitiator\t" << "send to " << mMessage->targetUUID();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    auto const outgoingFlow = mTrustLinesManager->outgoingFlow(
        mMessage->senderUUID);
    if (!maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
        outgoingFlowsForSending.push_back(
            outgoingFlow);
    }

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

const string MaxFlowCalculationTargetSndLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTargetSndLevelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

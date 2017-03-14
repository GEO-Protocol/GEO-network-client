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
        mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager) {}

InitiateMaxFlowCalculationMessage::Shared ReceiveMaxFlowCalculationOnTargetTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst ReceiveMaxFlowCalculationOnTargetTransaction::run() {

    info() << "run\t" << "target: " << mNodeUUID.stringUUID();
    info() << "run\t" << "initiator: " << mMessage->targetUUID().stringUUID();
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();

    sendResultToInitiator();
    sendMessagesOnFirstLevel();
    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendResultToInitiator() {

    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr
        = mMaxFlowCalculationCacheManager->cacheByNode(mMessage->senderUUID());
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }

    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlows;
    vector<pair<NodeUUID, TrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()) {
            incomingFlows.push_back(incomingFlow);
        }
    }

    sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderUUID(),
            mNodeUUID,
            outgoingFlows,
            incomingFlows);

    info() << "sendResultToInitiator\t" << "send to " << mMessage->senderUUID().stringUUID();

    info() << "sendResultToInitiator\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();

    auto maxFlowCalculationCache = make_shared<MaxFlowCalculationCache>(
        outgoingFlows,
        incomingFlows);

    mMaxFlowCalculationCacheManager->addCache(
            mMessage->senderUUID(),
            maxFlowCalculationCache);
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr) {

    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->senderUUID().stringUUID();

    info() << "sendCachedResultToInitiator\t" << "cache:";
    info() << "sendCachedResultToInitiator\t" << "outgoing: " << maxFlowCalculationCachePtr->mOutgoingFlows.size();
    for (auto const &it : maxFlowCalculationCachePtr->mOutgoingFlows) {
        info() << "sendCachedResultToInitiator\t" << "out uuid: " << it.first.stringUUID();
    }
    info() << "sendCachedResultToInitiator\t" << "incoming: " << maxFlowCalculationCachePtr->mIncomingFlows.size();
    for (auto const &it : maxFlowCalculationCachePtr->mIncomingFlows) {
        info() << "sendCachedResultToInitiator\t" << "in uuid: " << it.first.stringUUID();
    }

    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlowsForSending;

    vector<pair<NodeUUID, TrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(incomingFlow);
        }
    }
    info() << "sendCachedResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size();
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();

    for (auto const &it : outgoingFlowsForSending) {
        TrustLineAmount trustLineAmount = it.second;
        info() << "sendCachedResultToInitiator\t" << it.first.stringUUID();
    }

    if (incomingFlowsForSending.size() > 0) {

        sendMessage<ResultMaxFlowCalculationMessage>(
                mMessage->targetUUID(),
                mNodeUUID,
                outgoingFlowsForSending,
                incomingFlowsForSending);
    }

}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendMessagesOnFirstLevel() {

    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        if (nodeUUIDIncomingFlow == mMessage->targetUUID()) {
            continue;
        }

        info() << "sendFirst\t" << nodeUUIDIncomingFlow.stringUUID();

        sendMessage<MaxFlowCalculationTargetFstLevelMessage>(
                nodeUUIDIncomingFlow,
                mNodeUUID,
                mMessage->targetUUID());
    }

}

const string ReceiveMaxFlowCalculationOnTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveMaxFlowCalculationOnTargetTA]";

    return s.str();
}

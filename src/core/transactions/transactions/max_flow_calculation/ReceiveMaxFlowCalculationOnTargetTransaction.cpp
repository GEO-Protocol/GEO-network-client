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

    info() << "run\t" << "target: " << mNodeUUID;
    info() << "run\t" << "initiator: " << mMessage->senderUUID();
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

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()) {
            incomingFlows.push_back(
                incomingFlow);
        }
    }

    sendMessage<ResultMaxFlowCalculationMessage>(
        mMessage->senderUUID(),
        mNodeUUID,
        outgoingFlows,
        incomingFlows);

    info() << "sendResultToInitiator\t" << "send to " << mMessage->senderUUID();
    info() << "sendResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();

    mMaxFlowCalculationCacheManager->addCache(
        mMessage->senderUUID(),
        make_shared<MaxFlowCalculationCache>(
            outgoingFlows,
            incomingFlows));
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr) {

    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->senderUUID();

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(
                incomingFlow);
        }
    }
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();

    for (auto const &it : incomingFlowsForSending) {
        info() << "sendCachedResultToInitiator\t" << it.first;
    }

    if (incomingFlowsForSending.size() > 0) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderUUID(),
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendMessagesOnFirstLevel() {

    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        if (nodeUUIDIncomingFlow == mMessage->senderUUID()) {
            continue;
        }

        info() << "sendFirst\t" << nodeUUIDIncomingFlow;

        sendMessage<MaxFlowCalculationTargetFstLevelMessage>(
            nodeUUIDIncomingFlow,
            mNodeUUID,
            mMessage->senderUUID());
    }
}

const string ReceiveMaxFlowCalculationOnTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveMaxFlowCalculationOnTargetTA]";
    return s.str();
}

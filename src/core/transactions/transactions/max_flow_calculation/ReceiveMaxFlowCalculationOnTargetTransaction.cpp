#include "ReceiveMaxFlowCalculationOnTargetTransaction.h"

ReceiveMaxFlowCalculationOnTargetTransaction::ReceiveMaxFlowCalculationOnTargetTransaction(
        const NodeUUID &nodeUUID,
        InitiateMaxFlowCalculationMessage::Shared message,
        TrustLinesManager *manager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *logger) :

        BaseTransaction(
                BaseTransaction::TransactionType::ReceiveMaxFlowCalculationOnTargetTransactionType,
                nodeUUID),
        mMessage(message),
        mTrustLinesManager(manager),
        mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
        mLog(logger){}

InitiateMaxFlowCalculationMessage::Shared ReceiveMaxFlowCalculationOnTargetTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst ReceiveMaxFlowCalculationOnTargetTransaction::run() {

    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run", "target: " + mNodeUUID.stringUUID());
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run", "initiator: " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->outgoingFlows().size()));
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->incomingFlows().size()));

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

    Message *message = new ResultMaxFlowCalculationMessage(
        mNodeUUID,
        outgoingFlows,
        incomingFlows);

    addMessage(
        Message::Shared(message),
        mMessage->senderUUID()
    );
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendResultToInitiator",
                  "send to " + mMessage->senderUUID().stringUUID());

    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendResult",
                  "OutgoingFlows: " + to_string(outgoingFlows.size()));
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendResult",
                  "IncomingFlows: " + to_string(incomingFlows.size()));

    auto maxFlowCalculationCache = make_shared<MaxFlowCalculationCache>(
        mMessage->senderUUID(),
        outgoingFlows,
        incomingFlows);

    mMaxFlowCalculationCacheManager->addCache(maxFlowCalculationCache);
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr) {

    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                  "send to " + mMessage->senderUUID().stringUUID());

    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator", "cache:");
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                  "outgoing: " + to_string(maxFlowCalculationCachePtr->mOutgoingFlows.size()));
    for (auto const &it : maxFlowCalculationCachePtr->mOutgoingFlows) {
        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                      "out uuid: " + it.first.stringUUID());
    }
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                  "incoming: " + to_string(maxFlowCalculationCachePtr->mIncomingFlows.size()));
    for (auto const &it : maxFlowCalculationCachePtr->mIncomingFlows) {
        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                      "in uuid: " + it.first.stringUUID());
    }

    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlowsForSending;

    vector<pair<NodeUUID, TrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(incomingFlow);
        }
    }
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResult",
                  "OutgoingFlows: " + to_string(outgoingFlowsForSending.size()));
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResult",
                  "IncomingFlows: " + to_string(incomingFlowsForSending.size()));

    for (auto const &it : outgoingFlowsForSending) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResult", it.first.stringUUID());
    }

    if (incomingFlowsForSending.size() > 0) {
        Message *message = new ResultMaxFlowCalculationMessage(
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);

        addMessage(
            Message::Shared(message),
            mMessage->targetUUID());
    }

}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendMessagesOnFirstLevel() {

    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        NodeUUID targetUUID = mMessage->targetUUID();
        Message *message = new MaxFlowCalculationTargetFstLevelMessage(
            mNodeUUID,
            targetUUID);

        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendFirst",
                      ((NodeUUID)nodeUUIDIncomingFlow).stringUUID());
        addMessage(
            Message::Shared(message),
            nodeUUIDIncomingFlow);
    }

}

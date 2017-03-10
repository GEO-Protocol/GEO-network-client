#include "MaxFlowCalculationTargetSndLevelTransaction.h"

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetSndLevelMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetSndLevelTransactionType,
        nodeUUID
    ),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mLog(logger){}

MaxFlowCalculationTargetSndLevelMessage::Shared MaxFlowCalculationTargetSndLevelTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationTargetSndLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());

    sendResultToInitiator();

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

void MaxFlowCalculationTargetSndLevelTransaction::sendResultToInitiator() {

    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr
        = mMaxFlowCalculationCacheManager->cacheByNode(mMessage->targetUUID());
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }

    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first == mMessage->senderUUID()) {
            outgoingFlows.push_back(outgoingFlow);
        }
    }
    vector<pair<NodeUUID, TrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && incomingFlow.first != mMessage->targetUUID()) {
            incomingFlows.push_back(incomingFlow);
        }
    }

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResult",
                  "OutgoingFlows: " + to_string(outgoingFlows.size()));
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResult",
                  "IncomingFlows: " + to_string(incomingFlows.size()));

    Message *message = new ResultMaxFlowCalculationMessage(
        mNodeUUID,
        outgoingFlows,
        incomingFlows);

    addMessage(
        Message::Shared(message),
        mMessage->targetUUID());

    auto maxFlowCalculationCache = make_shared<MaxFlowCalculationCache>(
        mMessage->targetUUID(),
        outgoingFlows,
        incomingFlows);

    mMaxFlowCalculationCacheManager->addCache(maxFlowCalculationCache);
}

void MaxFlowCalculationTargetSndLevelTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr) {

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator", "cache:");
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                  "outgoing: " + to_string(maxFlowCalculationCachePtr->mOutgoingFlows.size()));
    for (auto const &it : maxFlowCalculationCachePtr->mOutgoingFlows) {
        mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                      "out uuid: " + it.first.stringUUID());
    }
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                  "incoming: " + to_string(maxFlowCalculationCachePtr->mIncomingFlows.size()));
    for (auto const &it : maxFlowCalculationCachePtr->mIncomingFlows) {
        mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                      "in uuid: " + it.first.stringUUID());
    }

    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlowsForSending;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first == mMessage->senderUUID()
            && !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
            outgoingFlowsForSending.push_back(outgoingFlow);
        }
    }
    vector<pair<NodeUUID, TrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && incomingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(incomingFlow);
        }
    }
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResult",
                  "OutgoingFlows: " + to_string(outgoingFlowsForSending.size()));
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResult",
                  "IncomingFlows: " + to_string(incomingFlowsForSending.size()));

    if (outgoingFlowsForSending.size() > 0 || incomingFlowsForSending.size() > 0) {
        Message *message = new ResultMaxFlowCalculationMessage(
            mNodeUUID,
            outgoingFlowsForSending,
            incomingFlowsForSending);

        addMessage(
            Message::Shared(message),
            mMessage->targetUUID());
    }
}

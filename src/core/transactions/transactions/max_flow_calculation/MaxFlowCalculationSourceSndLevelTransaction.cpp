#include "MaxFlowCalculationSourceSndLevelTransaction.h"

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceSndLevelMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceSndLevelTransactionType,
        nodeUUID
    ),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mLog(logger) {}

MaxFlowCalculationSourceSndLevelMessage::Shared MaxFlowCalculationSourceSndLevelTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationSourceSndLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());

    sendResultToInitiator();

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

void MaxFlowCalculationSourceSndLevelTransaction::sendResultToInitiator() {

    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr
        = mMaxFlowCalculationCacheManager->cacheByNode(mMessage->targetUUID());
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedResultToInitiator(maxFlowCalculationCachePtr);
        return;
    }

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());
    map<NodeUUID, TrustLineAmount> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID() && outgoingFlow.first != mMessage->targetUUID()) {
            outgoingFlows.insert(outgoingFlow);
        }
    }
    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first == mMessage->senderUUID()) {
            incomingFlows.insert(incomingFlow);
        }
    }
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResult",
                  "OutgoingFlows: " + to_string(outgoingFlows.size()));
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResult",
                  "IncomingFlows: " + to_string(incomingFlows.size()));

    /*for (auto const &it : outgoingFlows) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction::sendResult", it.first.stringUUID());
    }*/

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

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr) {

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator", "cache:");
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                  "outgoing: " + to_string(maxFlowCalculationCachePtr->mOutgoingFlows.size()));
    for (auto const &it : maxFlowCalculationCachePtr->mOutgoingFlows) {
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                      "out uuid: " + it.first.stringUUID());
    }
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                  "incoming: " + to_string(maxFlowCalculationCachePtr->mIncomingFlows.size()));
    for (auto const &it : maxFlowCalculationCachePtr->mIncomingFlows) {
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                      "in uuid: " + it.first.stringUUID());
    }

    map<NodeUUID, TrustLineAmount> outgoingFlowsForSending;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID()
            && outgoingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
            outgoingFlowsForSending.insert(outgoingFlow);
        }
    }
    map<NodeUUID, TrustLineAmount> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first == mMessage->senderUUID()
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.insert(incomingFlow);
        }
    }
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResult",
                  "OutgoingFlows: " + to_string(outgoingFlowsForSending.size()));
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResult",
                  "IncomingFlows: " + to_string(incomingFlowsForSending.size()));

    /*for (auto const &it : outgoingFlowsForSending) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction::sendCachedResult", it.first.stringUUID());
    }*/

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
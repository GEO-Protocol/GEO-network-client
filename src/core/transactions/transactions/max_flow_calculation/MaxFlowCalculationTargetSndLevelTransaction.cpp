#include "MaxFlowCalculationTargetSndLevelTransaction.h"

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetSndLevelInMessage::Shared message,
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

MaxFlowCalculationTargetSndLevelInMessage::Shared MaxFlowCalculationTargetSndLevelTransaction::message() const {

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

    if (mMaxFlowCalculationCacheManager->containsNodeUUID(mMessage->targetUUID())) {
        sendCachedResultToInitiator();
        return;
    }

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResult",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResult",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    map<NodeUUID, TrustLineAmount> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->getOutgoingFlows()) {
        if (outgoingFlow.first == mMessage->senderUUID()) {
            outgoingFlows.insert(outgoingFlow);
        }
    }
    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->getIncomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && incomingFlow.first != mMessage->targetUUID()) {
            incomingFlows.insert(incomingFlow);
        }
    }

    for (auto const &it : incomingFlows) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction::sendResult", it.first.stringUUID());
    }

    Message *message = new SendResultMaxFlowCalculationMessage(
        mNodeUUID,
        outgoingFlows,
        incomingFlows);

    addMessage(
        Message::Shared(message),
        mMessage->targetUUID());

    set<NodeUUID> outgoingCacheUUIDs;
    for (auto const &nodeUUIDAndFlow: outgoingFlows) {
        outgoingCacheUUIDs.insert(nodeUUIDAndFlow.first);
    }

    set<NodeUUID> incomingCacheUUIDs;
    for (auto const &nodeUUIDAndFlow: incomingFlows) {
        incomingCacheUUIDs.insert(nodeUUIDAndFlow.first);
    }

    auto maxFlowCalculationCache = make_shared<MaxFlowCalculationCache>(
        mMessage->targetUUID(),
        outgoingCacheUUIDs,
        incomingCacheUUIDs);

    mMaxFlowCalculationCacheManager->addCache(maxFlowCalculationCache);
}

void MaxFlowCalculationTargetSndLevelTransaction::sendCachedResultToInitiator() {

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());

    auto &maxFlowCalculatorCache = mMaxFlowCalculationCacheManager->mCaches.find(mMessage->targetUUID())->second;
    // todo cache out of date
    /*if (utc_now() - maxFlowCalculatorCache->mTimeStampCreated > 10000) {
        mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator", "out of date");
        return;
    }*/

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator", "cache:");
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                  "outgoing: " + to_string(maxFlowCalculatorCache->mOutgoingUUIDs.size()));
    for (auto const &it : maxFlowCalculatorCache->mOutgoingUUIDs) {
        mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                      "out uuid: " + it.stringUUID());
    }
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                  "incoming: " + to_string(maxFlowCalculatorCache->mIncomingUUIDs.size()));
    for (auto const &it : maxFlowCalculatorCache->mIncomingUUIDs) {
        mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResultToInitiator",
                      "in uuid: " + it.stringUUID());
    }

    map<NodeUUID, TrustLineAmount> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->getOutgoingFlows()) {
        if (outgoingFlow.first == mMessage->senderUUID()
            && !maxFlowCalculatorCache->containsOutgoingUUID(outgoingFlow.first)) {
            outgoingFlows.insert(outgoingFlow);
        }
    }
    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->getIncomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && incomingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculatorCache->containsIncomingUUID(incomingFlow.first)) {
            incomingFlows.insert(incomingFlow);
        }
    }
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResult",
                  "OutgoingFlows: " + to_string(outgoingFlows.size()));
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendCachedResult",
                  "IncomingFlows: " + to_string(incomingFlows.size()));

    for (auto const &it : outgoingFlows) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction::sendCachedResult", it.first.stringUUID());
    }

    if (outgoingFlows.size() > 0 || incomingFlows.size() > 0) {
        Message *message = new SendResultMaxFlowCalculationMessage(
            mNodeUUID,
            outgoingFlows,
            incomingFlows);

        addMessage(
            Message::Shared(message),
            mMessage->targetUUID());
    }
}

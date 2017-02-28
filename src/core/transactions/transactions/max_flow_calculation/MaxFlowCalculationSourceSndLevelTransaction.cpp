#include "MaxFlowCalculationSourceSndLevelTransaction.h"

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceSndLevelInMessage::Shared message,
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

MaxFlowCalculationSourceSndLevelInMessage::Shared MaxFlowCalculationSourceSndLevelTransaction::message() const {

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

    if (mMaxFlowCalculationCacheManager->containsNodeUUID(mMessage->targetUUID())) {
        sendCachedResultToInitiator();
        return;
    }

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());
    map<NodeUUID, TrustLineAmount> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->getOutgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID() && outgoingFlow.first != mMessage->targetUUID()) {
            outgoingFlows.insert(outgoingFlow);
        }
    }
    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->getIncomingFlows()) {
        if (incomingFlow.first == mMessage->senderUUID()) {
            incomingFlows.insert(incomingFlow);
        }
    }
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResult",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResult",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    for (auto const &it : outgoingFlows) {
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

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedResultToInitiator() {

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());

    auto &maxFlowCalculatorCache = mMaxFlowCalculationCacheManager->mCaches.find(mMessage->targetUUID())->second;
    // todo cache out of date
    /*if (utc_now() - maxFlowCalculatorCache->mTimeStampCreated > 10000) {
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator", "out of date");
        return;
    }*/

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator", "cache:");
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                  "outgoing: " + to_string(maxFlowCalculatorCache->mOutgoingUUIDs.size()));
    for (auto const &it : maxFlowCalculatorCache->mOutgoingUUIDs) {
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                      "out uuid: " + it.stringUUID());
    }
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                  "incoming: " + to_string(maxFlowCalculatorCache->mIncomingUUIDs.size()));
    for (auto const &it : maxFlowCalculatorCache->mIncomingUUIDs) {
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResultToInitiator",
                      "in uuid: " + it.stringUUID());
    }

    map<NodeUUID, TrustLineAmount> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->getOutgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID()
            && outgoingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculatorCache->containsOutgoingUUID(outgoingFlow.first)) {
            outgoingFlows.insert(outgoingFlow);
        }
    }
    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->getIncomingFlows()) {
        if (incomingFlow.first == mMessage->senderUUID()
            && !maxFlowCalculatorCache->containsIncomingUUID(incomingFlow.first)) {
            incomingFlows.insert(incomingFlow);
        }
    }
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResult",
                  "OutgoingFlows: " + to_string(outgoingFlows.size()));
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendCachedResult",
                  "IncomingFlows: " + to_string(incomingFlows.size()));

    for (auto const &it : outgoingFlows) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction::sendCachedResult", it.first.stringUUID());
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
#include "ReceiveMaxFlowCalculationOnTargetTransaction.h"

ReceiveMaxFlowCalculationOnTargetTransaction::ReceiveMaxFlowCalculationOnTargetTransaction(
        const NodeUUID &nodeUUID,
        ReceiveMaxFlowCalculationOnTargetMessage::Shared message,
        TrustLinesManager *manager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *logger) :

        BaseTransaction(
                BaseTransaction::TransactionType::ReceiveMaxFlowCalculationOnTargetTransactionType,
                nodeUUID
        ),
        mMessage(message),
        mTrustLinesManager(manager),
        mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
        mLog(logger){}

ReceiveMaxFlowCalculationOnTargetMessage::Shared ReceiveMaxFlowCalculationOnTargetTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst ReceiveMaxFlowCalculationOnTargetTransaction::run() {

    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run", "target: " + mNodeUUID.stringUUID());
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run", "initiator: " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    sendResultToInitiator();
    sendMessagesOnFirstLevel();
    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendResultToInitiator() {

    if (mMaxFlowCalculationCacheManager->containsNodeUUID(mMessage->senderUUID())) {
        sendCachedResultToInitiator();
        return;
    }

    map<NodeUUID, TrustLineAmount> outgoingFlows;
    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->getIncomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()) {
            incomingFlows.insert(incomingFlow);
        }
    }

    Message *message = new SendResultMaxFlowCalculationMessage(
        mNodeUUID,
        outgoingFlows,
        incomingFlows);

    addMessage(
        Message::Shared(message),
        mMessage->senderUUID()
    );
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendResultToInitiator",
                  "send to " + mMessage->senderUUID().stringUUID());

    set<NodeUUID> outgoingCacheUUIDs;

    set<NodeUUID> incomingCacheUUIDs;
    for (auto const &nodeUUIDAndFlow: incomingFlows) {
        incomingCacheUUIDs.insert(nodeUUIDAndFlow.first);
    }

    auto maxFlowCalculationCache = make_shared<MaxFlowCalculationCache>(
        mMessage->senderUUID(),
        outgoingCacheUUIDs,
        incomingCacheUUIDs);

    mMaxFlowCalculationCacheManager->addCache(maxFlowCalculationCache);
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResultToInitiator() {

    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                  "send to " + mMessage->senderUUID().stringUUID());

    auto &maxFlowCalculatorCache = mMaxFlowCalculationCacheManager->mCaches.find(mMessage->senderUUID())->second;
    // todo cache out of date
    /*if (utc_now() - maxFlowCalculatorCache->mTimeStampCreated > 10000) {
        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator", "out of date");
        return;
    }*/

    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator", "cache:");
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                  "outgoing: " + to_string(maxFlowCalculatorCache->mOutgoingUUIDs.size()));
    for (auto const &it : maxFlowCalculatorCache->mOutgoingUUIDs) {
        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                      "out uuid: " + it.stringUUID());
    }
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                  "incoming: " + to_string(maxFlowCalculatorCache->mIncomingUUIDs.size()));
    for (auto const &it : maxFlowCalculatorCache->mIncomingUUIDs) {
        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResultToInitiator",
                      "in uuid: " + it.stringUUID());
    }

    map<NodeUUID, TrustLineAmount> outgoingFlows;

    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->getIncomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && !maxFlowCalculatorCache->containsIncomingUUID(incomingFlow.first)) {
            incomingFlows.insert(incomingFlow);
        }
    }
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResult",
                  "OutgoingFlows: " + to_string(outgoingFlows.size()));
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendCachedResult",
                  "IncomingFlows: " + to_string(incomingFlows.size()));

    for (auto const &it : outgoingFlows) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResult", it.first.stringUUID());
    }

    if (incomingFlows.size() > 0) {
        Message *message = new SendResultMaxFlowCalculationMessage(
            mNodeUUID,
            outgoingFlows,
            incomingFlows);

        addMessage(
            Message::Shared(message),
            mMessage->targetUUID());
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendMessagesOnFirstLevel() {

    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->getFirstLevelNeighborsWithIncomingFlow();
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        NodeUUID targetUUID = mMessage->targetUUID();
        Message *message = new SendMaxFlowCalculationTargetFstLevelMessage(
            mNodeUUID,
            targetUUID);

        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendFirst",
                      ((NodeUUID)nodeUUIDIncomingFlow).stringUUID());
        addMessage(
            Message::Shared(message),
            nodeUUIDIncomingFlow);
    }

}

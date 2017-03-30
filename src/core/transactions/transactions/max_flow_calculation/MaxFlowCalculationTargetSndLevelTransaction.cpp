#include "MaxFlowCalculationTargetSndLevelTransaction.h"

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetSndLevelMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetSndLevelTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager) {}

MaxFlowCalculationTargetSndLevelMessage::Shared MaxFlowCalculationTargetSndLevelTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationTargetSndLevelTransaction::run() {

    info() << "run\t" << "Iam: " << mNodeUUID.stringUUID();
    info() << "run\t" << "sender: " << mMessage->senderUUID();
    info() << "run\t" << "target: " << mMessage->targetUUID();

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
            auto trustLineAmountShared = outgoingFlow.second;
            outgoingFlows.push_back(
                make_pair(
                    outgoingFlow.first,
                    *trustLineAmountShared.get()));
        }
    }
    vector<pair<NodeUUID, TrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()
            && incomingFlow.first != mMessage->targetUUID()) {
            auto trustLineAmountShared = incomingFlow.second;
            incomingFlows.push_back(
                make_pair(
                    incomingFlow.first,
                    *trustLineAmountShared.get()));
        }
    }

    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID();
    info() << "sendResultToInitiator\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();

    sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetUUID(),
            mNodeUUID,
            outgoingFlows,
            incomingFlows);

    auto maxFlowCalculationCache = make_shared<MaxFlowCalculationCache>(
        outgoingFlows,
        incomingFlows);

    mMaxFlowCalculationCacheManager->addCache(
            mMessage->targetUUID(),
            maxFlowCalculationCache);
}

void MaxFlowCalculationTargetSndLevelTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr) {

    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID();

    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlowsForSending;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        auto trustLineAmountShared = outgoingFlow.second;
        if (outgoingFlow.first == mMessage->senderUUID()
            && !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, *trustLineAmountShared.get())) {
            outgoingFlowsForSending.push_back(
                make_pair(
                    outgoingFlow.first,
                    *trustLineAmountShared.get()));
        }
    }
    vector<pair<NodeUUID, TrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        auto trustLineAmountShared = incomingFlow.second;
        if (incomingFlow.first != mMessage->senderUUID()
            && incomingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, *trustLineAmountShared.get())) {
            incomingFlowsForSending.push_back(
                make_pair(
                    incomingFlow.first,
                    *trustLineAmountShared.get()));
        }
    }
    info() << "sendCachedResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size();
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size();

    if (outgoingFlowsForSending.size() > 0 || incomingFlowsForSending.size() > 0) {

        sendMessage<ResultMaxFlowCalculationMessage>(
                mMessage->targetUUID(),
                mNodeUUID,
                outgoingFlowsForSending,
                incomingFlowsForSending);
    }
}

const string MaxFlowCalculationTargetSndLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTargetSndLevelTA]";

    return s.str();
}

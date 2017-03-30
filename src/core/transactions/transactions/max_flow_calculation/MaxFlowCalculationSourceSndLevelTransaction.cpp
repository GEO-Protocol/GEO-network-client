#include "MaxFlowCalculationSourceSndLevelTransaction.h"

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceSndLevelMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceSndLevelTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager) {}

MaxFlowCalculationSourceSndLevelMessage::Shared MaxFlowCalculationSourceSndLevelTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationSourceSndLevelTransaction::run() {

    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID();
    info() << "run\t" << "target: " << mMessage->targetUUID();

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

    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID();
    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID() && outgoingFlow.first != mMessage->targetUUID()) {
            auto trustLineAmountShared = outgoingFlow.second;
            outgoingFlows.push_back(
                make_pair(
                    outgoingFlow.first,
                    *trustLineAmountShared.get()));
        }
    }
    vector<pair<NodeUUID, TrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first == mMessage->senderUUID()) {
            auto trustLineAmountShared = incomingFlow.second;
            incomingFlows.push_back(
                make_pair(
                    incomingFlow.first,
                    *trustLineAmountShared.get()));
        }
    }
    info() << "sendResult\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResult\t" << "IncomingFlows: " << incomingFlows.size();

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

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedResultToInitiator(
    MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr) {

    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID();

    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlowsForSending;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        auto trustLineAmountShared = outgoingFlow.second;
        if (outgoingFlow.first != mMessage->senderUUID()
            && outgoingFlow.first != mMessage->targetUUID()
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
        if (incomingFlow.first == mMessage->senderUUID()
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

const string MaxFlowCalculationSourceSndLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationSourceSndLevelTA]";

    return s.str();
}
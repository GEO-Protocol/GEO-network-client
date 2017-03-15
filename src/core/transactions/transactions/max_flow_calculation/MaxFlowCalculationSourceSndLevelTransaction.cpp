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

    info() << "run\t" << "Iam: " << mNodeUUID.stringUUID();
    info() << "run\t" << "sender: " << mMessage->senderUUID().stringUUID();
    info() << "run\t" << "target: " << mMessage->targetUUID().stringUUID();

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

    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID().stringUUID();
    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID() && outgoingFlow.first != mMessage->targetUUID()) {
            outgoingFlows.push_back(outgoingFlow);
        }
    }
    vector<pair<NodeUUID, TrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first == mMessage->senderUUID()) {
            incomingFlows.push_back(incomingFlow);
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

    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID().stringUUID();

    info() << "sendCachedResultToInitiator\t" << "cache:";
    info() << "sendCachedResultToInitiator\t" << "outgoing: " << maxFlowCalculationCachePtr->mOutgoingFlows.size();
    for (auto const &it : maxFlowCalculationCachePtr->mOutgoingFlows) {
        info() << "sendCachedResultToInitiator\t" << "out uuid: " << it.first.stringUUID();
    }
    info() << "sendCachedResultToInitiator\t" << "incoming: " << maxFlowCalculationCachePtr->mIncomingFlows.size();
    for (auto const &it : maxFlowCalculationCachePtr->mIncomingFlows) {
        info() << "sendCachedResultToInitiator\t" << "in uuid: " + it.first.stringUUID();
    }

    vector<pair<NodeUUID, TrustLineAmount>> outgoingFlowsForSending;
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
        if (outgoingFlow.first != mMessage->senderUUID()
            && outgoingFlow.first != mMessage->targetUUID()
            && !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
            outgoingFlowsForSending.push_back(outgoingFlow);
        }
    }
    vector<pair<NodeUUID, TrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
        if (incomingFlow.first == mMessage->senderUUID()
            && !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(incomingFlow);
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
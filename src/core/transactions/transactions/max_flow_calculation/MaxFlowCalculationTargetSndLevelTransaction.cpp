#include "MaxFlowCalculationTargetSndLevelTransaction.h"

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetSndLevelMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    TopologyCacheManager *topologyCacheManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetSndLevelTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mTopologyCacheManager(topologyCacheManager),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst MaxFlowCalculationTargetSndLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID.stringUUID();
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "target: " << mMessage->targetUUID();
    info() << "run\t" << "i am is gateway: " << mIAmGateway;
#endif
    if (mIAmGateway) {
        sendGatewayResultToInitiator();
    } else {
        sendResultToInitiator();
    }
    return resultDone();
}

void MaxFlowCalculationTargetSndLevelTransaction::sendResultToInitiator()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
#endif
    TopologyCache::Shared maxFlowCalculationCachePtr
        = mTopologyCacheManager->cacheByAddress(mMessage->targetAddresses().at(0));
    TopologyCacheNew::Shared maxFlowCalculationCachePtrNew
        = mTopologyCacheManager->cacheByAddressNew(mMessage->targetAddresses().at(0));
    if (maxFlowCalculationCachePtr != nullptr and maxFlowCalculationCachePtrNew != nullptr) {
        sendCachedResultToInitiator(
            maxFlowCalculationCachePtr,
            maxFlowCalculationCachePtrNew);
        return;
    }
    if (maxFlowCalculationCachePtr != nullptr or maxFlowCalculationCachePtrNew != nullptr) {
        warning() << "Problem with cache!!!";
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    auto const outgoingFlow = mTrustLinesManager->outgoingFlow(
        mMessage->senderUUID);
    if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()) {
        outgoingFlows.push_back(
            outgoingFlow);
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsNew;
    auto const outgoingFlowNew = mTrustLinesManager->outgoingFlow(
        mMessage->idOnReceiverSide);
    if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()) {
        outgoingFlowsNew.push_back(
            outgoingFlowNew);
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromNonGateways()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
                incomingFlow.first != mMessage->senderUUID &&
                incomingFlow.first != mMessage->targetUUID()) {
            incomingFlows.push_back(
                incomingFlow);
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsNew;
    auto senderMainAddress = mContractorsManager->contractorMainAddress(mMessage->idOnReceiverSide);
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromNonGatewaysNew()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount()
            && incomingFlow.first != senderMainAddress
            && incomingFlow.first != mMessage->targetAddresses().at(0)) {
            incomingFlowsNew.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
    info() << "sendResultToInitiator\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size();
    info() << "sendResultToInitiator\t" << "OutgoingFlowsNew: " << outgoingFlowsNew.size();
    info() << "sendResultToInitiator\t" << "IncomingFlowsNew: " << incomingFlowsNew.size();
#endif
    if (!outgoingFlows.empty() || !incomingFlows.empty() ||
            !outgoingFlowsNew.empty() || !incomingFlowsNew.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetAddresses().at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            outgoingFlows,
            incomingFlows,
            outgoingFlowsNew,
            incomingFlowsNew);
        mTopologyCacheManager->addCache(
            mMessage->targetAddresses().at(0),
            make_shared<TopologyCache>(
                outgoingFlows,
                incomingFlows));
        mTopologyCacheManager->addCacheNew(
            mMessage->targetAddresses().at(0),
            make_shared<TopologyCacheNew>(
                outgoingFlowsNew,
                incomingFlowsNew));
    }
}

void MaxFlowCalculationTargetSndLevelTransaction::sendCachedResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr,
    TopologyCacheNew::Shared maxFlowCalculationCachePtrNew)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    auto const outgoingFlow = mTrustLinesManager->outgoingFlow(
        mMessage->senderUUID);
    if (*outgoingFlow.second.get() > TrustLine::kZeroAmount() &&
            !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
        outgoingFlowsForSending.push_back(
            outgoingFlow);
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsForSendingNew;
    auto const outgoingFlowNew = mTrustLinesManager->outgoingFlow(
        mMessage->idOnReceiverSide);
    if (!maxFlowCalculationCachePtrNew->containsOutgoingFlow(outgoingFlowNew.first, outgoingFlowNew.second)) {
        outgoingFlowsForSendingNew.push_back(
            outgoingFlowNew);
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromNonGateways()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
                incomingFlow.first != mMessage->senderUUID &&
                incomingFlow.first != mMessage->targetUUID() &&
                !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSending.push_back(
                incomingFlow);
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsForSendingNew;
    auto senderMainAddress = mContractorsManager->contractorMainAddress(mMessage->idOnReceiverSide);
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromNonGatewaysNew()) {
        if (incomingFlow.first != senderMainAddress
            && incomingFlow.first != mMessage->targetAddresses().at(0)
            && !maxFlowCalculationCachePtrNew->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSendingNew.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size()
           << " OutgoingFlowsNew " << outgoingFlowsForSendingNew.size();
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size()
           << " IncomingFlowsNew " << incomingFlowsForSendingNew.size();;
#endif
    if (!outgoingFlowsForSending.empty() || !incomingFlowsForSending.empty() ||
            !outgoingFlowsForSendingNew.empty() || !incomingFlowsForSendingNew.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->targetAddresses().at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            outgoingFlowsForSending,
            incomingFlowsForSending,
            outgoingFlowsForSendingNew,
            incomingFlowsForSendingNew);
    }
}

void MaxFlowCalculationTargetSndLevelTransaction::sendGatewayResultToInitiator()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
#endif
    TopologyCache::Shared maxFlowCalculationCachePtr
            = mTopologyCacheManager->cacheByAddress(mMessage->targetAddresses().at(0));
    TopologyCacheNew::Shared maxFlowCalculationCachePtrNew
            = mTopologyCacheManager->cacheByAddressNew(mMessage->targetAddresses().at(0));
    if (maxFlowCalculationCachePtr != nullptr and maxFlowCalculationCachePtrNew != nullptr) {
        sendCachedGatewayResultToInitiator(
            maxFlowCalculationCachePtr,
            maxFlowCalculationCachePtrNew);
        return;
    }

    if (maxFlowCalculationCachePtr != nullptr or maxFlowCalculationCachePtrNew != nullptr) {
        warning() << "Problem with cache!!!";
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    auto const outgoingFlow = mTrustLinesManager->outgoingFlow(
        mMessage->senderUUID);
    if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()) {
        outgoingFlows.push_back(
            outgoingFlow);
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsNew;
    auto const outgoingFlowNew = mTrustLinesManager->outgoingFlow(
            mMessage->idOnReceiverSide);
    if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()) {
        outgoingFlowsNew.push_back(
            outgoingFlowNew);
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    if (mMessage->isTargetGateway()) {
        for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromGateways()) {
            if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
                    incomingFlow.first != mMessage->senderUUID &&
                    incomingFlow.first != mMessage->targetUUID()) {
                incomingFlows.push_back(
                    incomingFlow);
            }
        }
    } else {
        for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
            if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
                    incomingFlow.first != mMessage->senderUUID &&
                    incomingFlow.first != mMessage->targetUUID()) {
                incomingFlows.push_back(
                    incomingFlow);
            }
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsNew;
    auto senderMainAddress = mContractorsManager->contractorMainAddress(mMessage->idOnReceiverSide);
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsNew()) {
        if (*incomingFlow.second.get() > TrustLine::kZeroAmount()
            && incomingFlow.first != senderMainAddress
            && incomingFlow.first != mMessage->targetAddresses().at(0)) {
            incomingFlowsNew.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendGatewayResultToInitiator\t" << "send to " << mMessage->targetUUID();
    info() << "sendGatewayResultToInitiator\t" << "OutgoingFlows: " << outgoingFlows.size()
           << " OutgoingFlowsNew " << outgoingFlowsNew.size();
    info() << "sendGatewayResultToInitiator\t" << "IncomingFlows: " << incomingFlows.size()
           << " IncomingFlowsNew " << incomingFlowsNew.size();
#endif
    if (!outgoingFlows.empty() || !incomingFlows.empty() ||
            !outgoingFlowsNew.empty() || !incomingFlowsNew.empty()) {
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
            mMessage->targetAddresses().at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            outgoingFlows,
            incomingFlows,
            outgoingFlowsNew,
            incomingFlowsNew);
        mTopologyCacheManager->addCache(
            mMessage->targetAddresses().at(0),
            make_shared<TopologyCache>(
                outgoingFlows,
                incomingFlows));
        mTopologyCacheManager->addCacheNew(
            mMessage->targetAddresses().at(0),
            make_shared<TopologyCacheNew>(
                outgoingFlowsNew,
                incomingFlowsNew));
    }
}

void MaxFlowCalculationTargetSndLevelTransaction::sendCachedGatewayResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr,
    TopologyCacheNew::Shared maxFlowCalculationCachePtrNew)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedGatewayResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    auto const outgoingFlow = mTrustLinesManager->outgoingFlow(
        mMessage->senderUUID);
    if (*outgoingFlow.second.get() > TrustLine::kZeroAmount() &&
            !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
        outgoingFlowsForSending.push_back(
            outgoingFlow);
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsForSendingNew;
    auto const outgoingFlowNew = mTrustLinesManager->outgoingFlow(
        mMessage->idOnReceiverSide);
    if (!maxFlowCalculationCachePtrNew->containsOutgoingFlow(outgoingFlowNew.first, outgoingFlowNew.second)) {
        outgoingFlowsForSendingNew.push_back(
            outgoingFlowNew);
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    if (mMessage->isTargetGateway()) {
        for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromGateways()) {
            if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
                    incomingFlow.first != mMessage->senderUUID &&
                    incomingFlow.first != mMessage->targetUUID() &&
                    !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
                incomingFlowsForSending.push_back(
                    incomingFlow);
            }
        }
    } else {
        for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
            if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
                    incomingFlow.first != mMessage->senderUUID &&
                    incomingFlow.first != mMessage->targetUUID() &&
                    !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
                incomingFlowsForSending.push_back(
                    incomingFlow);
            }
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsForSendingNew;
    auto senderMainAddress = mContractorsManager->contractorMainAddress(mMessage->idOnReceiverSide);
    for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsNew()) {
        if (incomingFlow.first != senderMainAddress
            && incomingFlow.first != mMessage->targetAddresses().at(0)
            && !maxFlowCalculationCachePtrNew->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
            incomingFlowsForSendingNew.push_back(
                incomingFlow);
        }
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedGatewayResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size()
           << " OutgoingFlowsNew: " << outgoingFlowsForSendingNew.size();
    info() << "sendCachedGatewayResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size()
           << " IncomingFlowsNew: " << incomingFlowsForSendingNew.size();
#endif
    if (!outgoingFlowsForSending.empty() || !incomingFlowsForSending.empty() ||
            !outgoingFlowsForSendingNew.empty() || !incomingFlowsForSendingNew.empty()) {
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
            mMessage->targetAddresses().at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            outgoingFlowsForSending,
            incomingFlowsForSending,
            outgoingFlowsForSendingNew,
            incomingFlowsForSendingNew);
    }
}

const string MaxFlowCalculationTargetSndLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTargetSndLevelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

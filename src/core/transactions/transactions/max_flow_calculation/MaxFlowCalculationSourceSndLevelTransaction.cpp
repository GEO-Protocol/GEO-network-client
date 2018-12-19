#include "MaxFlowCalculationSourceSndLevelTransaction.h"

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceSndLevelMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    TopologyCacheManager *topologyCacheManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceSndLevelTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager),
    mTopologyCacheManager(topologyCacheManager),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst MaxFlowCalculationSourceSndLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "i am is gateway: " << mIAmGateway;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "target: " << mMessage->targetUUID();
#endif
    if (mIAmGateway) {
        sendGatewayResultToInitiator();
    } else {
        sendResultToInitiator();
    }
    return resultDone();
}

void MaxFlowCalculationSourceSndLevelTransaction::sendResultToInitiator()
{
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
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    bool isSourceFirstLevelNode = false;
    if (mTrustLinesManager->trustLineIsPresent(mMessage->targetUUID()) and
            *mTrustLinesManager->incomingFlow(mMessage->targetUUID()).second > TrustLine::kZeroAmount()) {
        isSourceFirstLevelNode = true;
    }
    if (!isSourceFirstLevelNode) {
        for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
            if (*outgoingFlow.second.get() > TrustLine::kZeroAmount() &&
                    outgoingFlow.first != mMessage->senderUUID &&
                    outgoingFlow.first != mMessage->targetUUID()) {
                outgoingFlows.push_back(
                    outgoingFlow);
            }
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsNew;
    auto senderMainAddress = mContractorsManager->contractorMainAddress(mMessage->idOnReceiverSide);
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlowsNew()) {
        if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()
            && outgoingFlow.first != senderMainAddress
            && outgoingFlow.first != mMessage->targetAddresses().at(0)) {
            outgoingFlowsNew.push_back(
                outgoingFlow);
        }
    }
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    const auto incomingFlow = mTrustLinesManager->incomingFlow(mMessage->senderUUID);
    if (*incomingFlow.second.get() > TrustLine::kZeroAmount()) {
        incomingFlows.push_back(
            incomingFlow);
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsNew;
    const auto incomingFlowNew = mTrustLinesManager->incomingFlow(mMessage->idOnReceiverSide);
    if (*incomingFlow.second.get() > TrustLine::kZeroAmount()) {
        incomingFlowsNew.push_back(
            incomingFlowNew);
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendResult\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendResult\t" << "IncomingFlows: " << incomingFlows.size();
    info() << "sendResult\t" << "OutgoingFlowsNew: " << outgoingFlowsNew.size();
    info() << "sendResult\t" << "IncomingFlowsNew: " << incomingFlowsNew.size();
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

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr,
    TopologyCacheNew::Shared maxFlowCalculationCachePtrNew)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    bool isSourceFirstLevelNode = false;
    if (mTrustLinesManager->trustLineIsPresent(mMessage->targetUUID()) and
        *mTrustLinesManager->incomingFlow(mMessage->targetUUID()).second > TrustLine::kZeroAmount()) {
        isSourceFirstLevelNode = true;
    }
    if (!isSourceFirstLevelNode) {
        for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlows()) {
            if (*outgoingFlow.second.get() > TrustLine::kZeroAmount() &&
                    outgoingFlow.first != mMessage->senderUUID &&
                    outgoingFlow.first != mMessage->targetUUID() &&
                    !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
                outgoingFlowsForSending.push_back(
                    outgoingFlow);
            }
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsForSendingNew;
    auto senderMainAddress = mContractorsManager->contractorMainAddress(mMessage->idOnReceiverSide);
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlowsNew()) {
        if (outgoingFlow.first != senderMainAddress
            && outgoingFlow.first != mMessage->targetAddresses().at(0)
            && !maxFlowCalculationCachePtrNew->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
            outgoingFlowsForSendingNew.push_back(
                outgoingFlow);
        }
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    auto const incomingFlow = mTrustLinesManager->incomingFlow(mMessage->senderUUID);
    if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
            !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
        incomingFlowsForSending.push_back(
            incomingFlow);
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsForSendingNew;
    auto const incomingFlowNew = mTrustLinesManager->incomingFlow(mMessage->idOnReceiverSide);
    if (!maxFlowCalculationCachePtrNew->containsIncomingFlow(incomingFlowNew.first, incomingFlowNew.second)) {
        incomingFlowsForSendingNew.push_back(
            incomingFlowNew);
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator\t" << "OutgoingFlows: " << outgoingFlowsForSending.size()
           << " OutgoingFlowsNew: " << outgoingFlowsForSendingNew.size();
    info() << "sendCachedResultToInitiator\t" << "IncomingFlows: " << incomingFlowsForSending.size()
           << " IncomingFlowsNew: " << incomingFlowsForSendingNew.size();
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

void MaxFlowCalculationSourceSndLevelTransaction::sendGatewayResultToInitiator()
{
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
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendGatewayResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
    bool isSourceFirstLevelNode = false;
    if (mTrustLinesManager->trustLineIsPresent(mMessage->targetUUID()) and
        *mTrustLinesManager->incomingFlow(mMessage->targetUUID()).second > TrustLine::kZeroAmount()) {
        isSourceFirstLevelNode = true;
    }
    if (!isSourceFirstLevelNode) {
        for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlowsToGateways()) {
            if (*outgoingFlow.second.get() > TrustLine::kZeroAmount() &&
                    outgoingFlow.first != mMessage->senderUUID &&
                    outgoingFlow.first != mMessage->targetUUID()) {
                outgoingFlows.push_back(
                    outgoingFlow);
            }
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsNew;
    auto senderMainAddress = mContractorsManager->contractorMainAddress(mMessage->idOnReceiverSide);
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlowsToGatewaysNew()) {
        if (*outgoingFlow.second.get() > TrustLine::kZeroAmount()
            && outgoingFlow.first != senderMainAddress
            && outgoingFlow.first != mMessage->targetAddresses().at(0)) {
            outgoingFlowsNew.push_back(
                outgoingFlow);
        }
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
    const auto incomingFlow = mTrustLinesManager->incomingFlow(mMessage->senderUUID);
    if (*incomingFlow.second.get() > TrustLine::kZeroAmount()) {
        incomingFlows.push_back(
            incomingFlow);
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsNew;
    const auto incomingFlowNew = mTrustLinesManager->incomingFlow(mMessage->idOnReceiverSide);
    if (*incomingFlow.second.get() > TrustLine::kZeroAmount()) {
        incomingFlowsNew.push_back(
            incomingFlowNew);
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendGatewayResult\t" << "OutgoingFlows: " << outgoingFlows.size();
    info() << "sendGatewayResult\t" << "IncomingFlows: " << incomingFlows.size();
    info() << "sendGatewayResult\t" << "OutgoingFlowsNew: " << outgoingFlowsNew.size();
    info() << "sendGatewayResult\t" << "IncomingFlowsNew: " << incomingFlowsNew.size();
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

void MaxFlowCalculationSourceSndLevelTransaction::sendCachedGatewayResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr,
    TopologyCacheNew::Shared maxFlowCalculationCachePtrNew)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedGatewayResultToInitiator\t" << "send to " << mMessage->targetUUID()
           << " " << mMessage->targetAddresses().at(0)->fullAddress();
#endif
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    bool isSourceFirstLevelNode = false;
    if (mTrustLinesManager->trustLineIsPresent(mMessage->targetUUID()) and
        *mTrustLinesManager->incomingFlow(mMessage->targetUUID()).second > TrustLine::kZeroAmount()) {
        isSourceFirstLevelNode = true;
    }
    if (!isSourceFirstLevelNode) {
        for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlowsToGateways()) {
            if (*outgoingFlow.second.get() > TrustLine::kZeroAmount() &&
                    outgoingFlow.first != mMessage->senderUUID &&
                    outgoingFlow.first != mMessage->targetUUID() &&
                    !maxFlowCalculationCachePtr->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
                outgoingFlowsForSending.push_back(
                    outgoingFlow);
            }
        }
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsForSendingNew;
    auto senderMainAddress = mContractorsManager->contractorMainAddress(mMessage->idOnReceiverSide);
    for (auto const &outgoingFlow : mTrustLinesManager->outgoingFlowsToGatewaysNew()) {
        if (outgoingFlow.first != senderMainAddress
            && outgoingFlow.first != mMessage->targetAddresses().at(0)
            && !maxFlowCalculationCachePtrNew->containsOutgoingFlow(outgoingFlow.first, outgoingFlow.second)) {
            outgoingFlowsForSendingNew.push_back(
                outgoingFlow);
        }
    }

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    auto const incomingFlow = mTrustLinesManager->incomingFlow(mMessage->senderUUID);
    if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
            !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
        incomingFlowsForSending.push_back(
            incomingFlow);
    }
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsForSendingNew;
    auto const incomingFlowNew = mTrustLinesManager->incomingFlow(mMessage->idOnReceiverSide);
    if (!maxFlowCalculationCachePtrNew->containsIncomingFlow(incomingFlowNew.first, incomingFlowNew.second)) {
        incomingFlowsForSendingNew.push_back(
            incomingFlowNew);
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

const string MaxFlowCalculationSourceSndLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationSourceSndLevelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
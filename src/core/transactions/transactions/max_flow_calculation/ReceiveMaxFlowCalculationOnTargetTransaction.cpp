#include "ReceiveMaxFlowCalculationOnTargetTransaction.h"

ReceiveMaxFlowCalculationOnTargetTransaction::ReceiveMaxFlowCalculationOnTargetTransaction(
    InitiateMaxFlowCalculationMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::ReceiveMaxFlowCalculationOnTargetTransactionType,
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mTopologyCacheManager(topologyCacheManager)
{}

TransactionResult::SharedConst ReceiveMaxFlowCalculationOnTargetTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "initiator: " << mMessage->senderAddresses.at(0)->fullAddress();
#endif
    sendResultToInitiator();
    sendMessagesOnFirstLevel();
    return resultDone();
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendResultToInitiator()
{
    TopologyCache::Shared maxFlowCalculationCachePtr = mTopologyCacheManager->cacheByAddress(
        mMessage->senderAddresses.at(0));
    if (maxFlowCalculationCachePtr != nullptr) {
        sendCachedResultToInitiator(
            maxFlowCalculationCachePtr);
        return;
    }

    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlows;
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlows;
    if (mMessage->isSenderGateway()) {
        for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromGateways()) {
            if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
                    incomingFlow.first != mMessage->senderAddresses.at(0)) {
                incomingFlows.push_back(
                    incomingFlow);
            }
        }
    } else {
        for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
            if (*incomingFlow.second.get() > TrustLine::kZeroAmount() &&
                    incomingFlow.first != mMessage->senderAddresses.at(0)) {
                incomingFlows.push_back(
                    incomingFlow);
            }
        }
    }

#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "IncomingFlows: " << incomingFlows.size();
#endif
    if (!incomingFlows.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderAddresses.at(0),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            outgoingFlows,
            incomingFlows);
        mTopologyCacheManager->addCache(
            mMessage->senderAddresses.at(0),
            make_shared<TopologyCache>(
                outgoingFlows,
                incomingFlows));
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendCachedResultToInitiator(
    TopologyCache::Shared maxFlowCalculationCachePtr)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "sendCachedResultToInitiator";
#endif
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsForSending;
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsForSending;
    if (mMessage->isSenderGateway()) {
        for (auto const &incomingFlow : mTrustLinesManager->incomingFlowsFromGateways()) {
            if (incomingFlow.first != mMessage->senderAddresses.at(0) &&
                    !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
                incomingFlowsForSending.push_back(
                    incomingFlow);
            }
        }
    } else {
        for (auto const &incomingFlow : mTrustLinesManager->incomingFlows()) {
            if (incomingFlow.first != mMessage->senderAddresses.at(0) &&
                    !maxFlowCalculationCachePtr->containsIncomingFlow(incomingFlow.first, incomingFlow.second)) {
                incomingFlowsForSending.push_back(
                    incomingFlow);
            }
        }
    }

#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "IncomingFlows: " << incomingFlowsForSending.size();
#endif
    if (!incomingFlowsForSending.empty()) {
        sendMessage<ResultMaxFlowCalculationMessage>(
            mMessage->senderAddresses.at(0),
            mEquivalent,
            mContractorsManager->ownAddresses(),
            outgoingFlowsForSending,
            incomingFlowsForSending);
    }
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendMessagesOnFirstLevel()
{
    vector<ContractorID> incomingFlowIDs;
    if (mMessage->isSenderGateway()) {
        incomingFlowIDs = mTrustLinesManager->firstLevelGatewayNeighborsWithIncomingFlow();
    } else {
        incomingFlowIDs = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    }
    auto initiatorContractorID = mContractorsManager->contractorIDByAddress(mMessage->senderAddresses.at(0));

    for (auto const &nodeIDWithIncomingFlow : incomingFlowIDs) {
        if (nodeIDWithIncomingFlow == initiatorContractorID) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeIDWithIncomingFlow;
#endif
        sendMessage<MaxFlowCalculationTargetFstLevelMessage>(
            nodeIDWithIncomingFlow,
            mEquivalent,
            mContractorsManager->idOnContractorSide(nodeIDWithIncomingFlow),
            mMessage->senderAddresses,
            mMessage->isSenderGateway());
    }
}

const string ReceiveMaxFlowCalculationOnTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveMaxFlowCalculationOnTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

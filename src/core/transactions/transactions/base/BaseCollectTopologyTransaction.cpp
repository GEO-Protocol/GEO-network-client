#include "BaseCollectTopologyTransaction.h"

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLinesManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &logger) :

    BaseTransaction(
        type,
        nodeUUID,
        logger),
    mTrustLinesManager(trustLinesManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager)
{}

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLinesManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &logger) :

    BaseTransaction(
        type,
        transactionUUID,
        nodeUUID,
        logger),
    mTrustLinesManager(trustLinesManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager)
{}

TransactionResult::SharedConst BaseCollectTopologyTransaction::run()
{
    switch (mStep) {
        case Stages::SendRequestForCollectingTopology: {
            mStep = Stages::ProcessCollectingTopology;
            return sendRequestForCollectingTopology();
        }
        case Stages::ProcessCollectingTopology: {
            return processCollectingTopology();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                                 "wrong value of mStep");
    }
}

void BaseCollectTopologyTransaction::fillTopology()
{
    while (!mContext.empty()) {
        if (mContext.at(0)->typeID() != Message::MaxFlow_ResultMaxFlowCalculation) {
            warning() << "Invalid message type in context during fill topology";
            mContext.pop_front();
            continue;
        }
        const auto kMessage = popNextMessage<ResultMaxFlowCalculationMessage>();
        for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
            mMaxFlowCalculationTrustLineManager->addTrustLine(
                make_shared<MaxFlowCalculationTrustLine>(
                    kMessage->senderUUID,
                    outgoingFlow.first,
                    outgoingFlow.second));
        }
        for (auto const &incomingFlow : kMessage->incomingFlows()) {
            mMaxFlowCalculationTrustLineManager->addTrustLine(
                make_shared<MaxFlowCalculationTrustLine>(
                    incomingFlow.first,
                    kMessage->senderUUID,
                    incomingFlow.second));
        }
    }
}

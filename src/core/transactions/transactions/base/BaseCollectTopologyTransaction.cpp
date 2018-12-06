#include "BaseCollectTopologyTransaction.h"

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &logger) :

    BaseTransaction(
        type,
        nodeUUID,
        equivalent,
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{}

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &logger) :

    BaseTransaction(
        type,
        transactionUUID,
        nodeUUID,
        equivalent,
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
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
        case Stages::CustomLogic:
            return applyCustomLogic();
        default:
            throw ValueError(logHeader() + "::run: "
                                 "wrong value of mStep");
    }
}

void BaseCollectTopologyTransaction::fillTopology()
{
    while (!mContext.empty()) {
        if (mContext.at(0)->typeID() == Message::MaxFlow_ResultMaxFlowCalculation) {
            const auto kMessage = popNextMessage<ResultMaxFlowCalculationMessage>();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "Sender " << kMessage->senderUUID << " common";
            info() << "ConfirmationID " << kMessage->confirmationID();
#endif
            for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        kMessage->senderUUID,
                        outgoingFlow.first,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlows()) {
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        incomingFlow.first,
                        kMessage->senderUUID,
                        incomingFlow.second));
            }

            auto senderID = mTopologyTrustLineManager->getID(kMessage->senderAddresses.at(0));
            for (auto const &outgoingFlow : kMessage->outgoingFlowsNew()) {
                auto targetID = mTopologyTrustLineManager->getID(outgoingFlow.first);
                mTopologyTrustLineManager->addTrustLineNew(
                    make_shared<TopologyTrustLineNew>(
                        senderID,
                        targetID,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlowsNew()) {
                auto sourceID = mTopologyTrustLineManager->getID(incomingFlow.first);
                mTopologyTrustLineManager->addTrustLineNew(
                    make_shared<TopologyTrustLineNew>(
                        sourceID,
                        senderID,
                        incomingFlow.second));
            }
        }
        else if (mContext.at(0)->typeID() == Message::MaxFlow_ResultMaxFlowCalculationFromGateway) {
            const auto kMessage = popNextMessage<ResultMaxFlowCalculationGatewayMessage>();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "Sender " << kMessage->senderUUID << " gateway";
#endif
            mTopologyTrustLineManager->addGateway(kMessage->senderUUID);
            for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        kMessage->senderUUID,
                        outgoingFlow.first,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlows()) {
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                         incomingFlow.first,
                         kMessage->senderUUID,
                         incomingFlow.second));
            }

            auto senderID = mTopologyTrustLineManager->getID(kMessage->senderAddresses.at(0));
            mTopologyTrustLineManager->addGatewayNew(senderID);
            for (auto const &outgoingFlow : kMessage->outgoingFlowsNew()) {
                auto targetID = mTopologyTrustLineManager->getID(outgoingFlow.first);
                mTopologyTrustLineManager->addTrustLineNew(
                    make_shared<TopologyTrustLineNew>(
                        senderID,
                        targetID,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlowsNew()) {
                auto sourceID = mTopologyTrustLineManager->getID(incomingFlow.first);
                mTopologyTrustLineManager->addTrustLineNew(
                    make_shared<TopologyTrustLineNew>(
                        sourceID,
                        senderID,
                        incomingFlow.second));
            }

            mGateways.insert(
                kMessage->senderUUID);
            mGatewaysNew.insert(
                senderID);
        }
        else {
            warning() << "Invalid message type in context during fill topology";
            mContext.pop_front();
        }
    }
}

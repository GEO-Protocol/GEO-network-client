#include "BaseCollectTopologyTransaction.h"

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    TailManager &tailManager,
    Logger &logger) :

    BaseTransaction(
        type,
        equivalent,
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mTailManager(tailManager)
{}

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    TailManager &tailManager,
    Logger &logger) :

    BaseTransaction(
        type,
        transactionUUID,
        equivalent,
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mTailManager(tailManager)
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
    /// Take messages from TailManager instead of BaseTransaction's 'mContext'
    auto &mContext = mTailManager.getTail(Message::MaxFlow_ResultMaxFlowCalculation);

    while (!mContext.empty()) {
        if (mContext.front()->typeID() == Message::MaxFlow_ResultMaxFlowCalculation) {
            const auto kMessage = popNextMessage<ResultMaxFlowCalculationMessage>(mContext);
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "Sender " << kMessage->senderAddresses.at(0)->fullAddress() << " common";
            info() << "Outgoing flows: " << kMessage->outgoingFlows().size();
            info() << "Incoming flows: " << kMessage->incomingFlows().size();
            info() << "ConfirmationID " << kMessage->confirmationID();
#endif
            auto senderID = mTopologyTrustLineManager->getID(kMessage->senderAddresses.at(0));
            for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
                auto targetID = mTopologyTrustLineManager->getID(outgoingFlow.first);
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        senderID,
                        targetID,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlows()) {
                auto sourceID = mTopologyTrustLineManager->getID(incomingFlow.first);
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        sourceID,
                        senderID,
                        incomingFlow.second));
            }
        }
        else if (mContext.front()->typeID() == Message::MaxFlow_ResultMaxFlowCalculationFromGateway) {
            const auto kMessage = popNextMessage<ResultMaxFlowCalculationGatewayMessage>(mContext);
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "Sender " << kMessage->senderAddresses.at(0)->fullAddress() << " gateway";
            info() << "Outgoing flows: " << kMessage->outgoingFlows().size();
            info() << "Incoming flows: " << kMessage->incomingFlows().size();
            info() << "ConfirmationID " << kMessage->confirmationID();
#endif
            auto senderID = mTopologyTrustLineManager->getID(kMessage->senderAddresses.at(0));
            mTopologyTrustLineManager->addGateway(senderID);
            for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
                auto targetID = mTopologyTrustLineManager->getID(outgoingFlow.first);
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        senderID,
                        targetID,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlows()) {
                auto sourceID = mTopologyTrustLineManager->getID(incomingFlow.first);
                mTopologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        sourceID,
                        senderID,
                        incomingFlow.second));
            }

            mGateways.insert(
                senderID);
        }
        else {
            warning() << "Invalid message type in context during fill topology";
            mContext.pop_front();
        }
    }
}

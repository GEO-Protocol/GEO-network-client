#include "BaseCollectTopologyTransaction.h"

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    TailManager *tailManager,
    Logger &logger) :

    BaseTransaction(
        type,
        equivalent,
        logger),
    mContractorsManager(contractorsManager),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mTrustLinesManager(equivalentsSubsystemsRouter->trustLinesManager(equivalent)),
    mTopologyTrustLineManager(equivalentsSubsystemsRouter->topologyTrustLineManager(equivalent)),
    mTopologyCacheManager(equivalentsSubsystemsRouter->topologyCacheManager(equivalent)),
    mMaxFlowCacheManager(equivalentsSubsystemsRouter->maxFlowCacheManager(equivalent)),
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
    auto &mContext = mTailManager->getFlowTail();

    while (!mContext.empty()) {
        if (mContext.front()->typeID() == Message::MaxFlow_ResultMaxFlowCalculation) {
            const auto kMessage = popNextMessage<ResultMaxFlowCalculationMessage>(mContext);
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            debug() << "Equivalent " << kMessage->equivalent();
            debug() << "Sender " << kMessage->senderAddresses.at(0)->fullAddress() << " common";
            debug() << "Outgoing flows: " << kMessage->outgoingFlows().size();
            debug() << "Incoming flows: " << kMessage->incomingFlows().size();
            debug() << "ConfirmationID " << kMessage->confirmationID();
#endif
            auto topologyTrustLineManager = mEquivalentsSubsystemsRouter->topologyTrustLineManager(
                kMessage->equivalent());
            auto senderID = topologyTrustLineManager->getID(kMessage->senderAddresses.at(0));
            for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
                auto targetID = mTopologyTrustLineManager->getID(outgoingFlow.first);
                topologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        senderID,
                        targetID,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlows()) {
                auto sourceID = topologyTrustLineManager->getID(incomingFlow.first);
                topologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        sourceID,
                        senderID,
                        incomingFlow.second));
            }
        }
        else if (mContext.front()->typeID() == Message::MaxFlow_ResultMaxFlowCalculationFromGateway) {
            const auto kMessage = popNextMessage<ResultMaxFlowCalculationGatewayMessage>(mContext);
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            debug() << "Equivalent " << kMessage->equivalent();
            debug() << "Sender " << kMessage->senderAddresses.at(0)->fullAddress() << " gateway";
            debug() << "Outgoing flows: " << kMessage->outgoingFlows().size();
            debug() << "Incoming flows: " << kMessage->incomingFlows().size();
            debug() << "ConfirmationID " << kMessage->confirmationID();
#endif
            auto topologyTrustLineManager = mEquivalentsSubsystemsRouter->topologyTrustLineManager(
                kMessage->equivalent());
            auto senderID = topologyTrustLineManager->getID(kMessage->senderAddresses.at(0));
            topologyTrustLineManager->addGateway(senderID);
            for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
                auto targetID = topologyTrustLineManager->getID(outgoingFlow.first);
                topologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        senderID,
                        targetID,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlows()) {
                auto sourceID = topologyTrustLineManager->getID(incomingFlow.first);
                topologyTrustLineManager->addTrustLine(
                    make_shared<TopologyTrustLine>(
                        sourceID,
                        senderID,
                        incomingFlow.second));
            }

            if (kMessage->equivalent() == mEquivalent) {
                mGateways.insert(
                    senderID);
            }
        }
        else {
            warning() << "Invalid message type in context during fill topology";
            mContext.pop_front();
        }
    }
}

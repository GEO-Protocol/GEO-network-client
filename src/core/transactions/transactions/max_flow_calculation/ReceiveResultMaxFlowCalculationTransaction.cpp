#include "ReceiveResultMaxFlowCalculationTransaction.h"

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mSenderIsGateway(false)
{}

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationGatewayMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mSenderIsGateway(true)
{}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "initiator: " << mNodeUUID;
    info() << "sender: " << mMessage->senderUUID;
    info() << "sender is gateway: " << mSenderIsGateway;
    info() << "beforeInsert mapTrustLinesCount: " << mTopologyTrustLineManager->trustLinesCounts();
    info() << "receivedTrustLinesOut: " << mMessage->outgoingFlows().size();
#endif

    auto senderID = mTopologyTrustLineManager->getID(mMessage->senderAddresses.at(0));
    if (mSenderIsGateway) {
        mTopologyTrustLineManager->addGateway(
            mMessage->senderUUID);
        mTopologyTrustLineManager->addGatewayNew(senderID);
    }

    for (auto const &outgoingFlow : mMessage->outgoingFlows()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "\t" << outgoingFlow.first << " " << *outgoingFlow.second.get();
#endif
        mTopologyTrustLineManager->addTrustLine(
            make_shared<TopologyTrustLine>(
                mMessage->senderUUID,
                outgoingFlow.first,
                outgoingFlow.second));
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "receivedTrustLinesIn: " << mMessage->incomingFlows().size();
#endif
    for (auto const &incomingFlow : mMessage->incomingFlows()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "\t" << incomingFlow.first << " " << *incomingFlow.second.get();
#endif
        mTopologyTrustLineManager->addTrustLine(
            make_shared<TopologyTrustLine>(
                incomingFlow.first,
                mMessage->senderUUID,
                incomingFlow.second));
    }
    ////////////////////////////////////////////////////////
    info() << "receivedTrustLinesOutNew: " << mMessage->outgoingFlowsNew().size();
    for (auto const &outgoingFlow : mMessage->outgoingFlowsNew()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "\t" << outgoingFlow.first->fullAddress() << " " << *outgoingFlow.second.get();
#endif
        auto targetID = mTopologyTrustLineManager->getID(outgoingFlow.first);
        mTopologyTrustLineManager->addTrustLineNew(
            make_shared<TopologyTrustLineNew>(
                senderID,
                targetID,
                outgoingFlow.second));
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "receivedTrustLinesInNew: " << mMessage->incomingFlowsNew().size();
#endif
    for (auto const &incomingFlow : mMessage->incomingFlowsNew()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "\t" << incomingFlow.first->fullAddress() << " " << *incomingFlow.second.get();
#endif
        auto sourceID = mTopologyTrustLineManager->getID(incomingFlow.first);
        mTopologyTrustLineManager->addTrustLineNew(
            make_shared<TopologyTrustLineNew>(
                sourceID,
                senderID,
                incomingFlow.second));
    }
    return resultDone();
}

const string ReceiveResultMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveResultMaxFlowCalculationTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}


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

ResultMaxFlowCalculationMessage::Shared ReceiveResultMaxFlowCalculationTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "initiator: " << mNodeUUID;
    info() << "sender: " << mMessage->senderUUID;
    info() << "sender is gateway: " << mSenderIsGateway;
    info() << "beforeInsert mapTrustLinesCount: " << mTopologyTrustLineManager->trustLinesCounts();
    info() << "receivedTrustLinesOut: " << mMessage->outgoingFlows().size();
#endif

    if (mSenderIsGateway) {
        mTopologyTrustLineManager->addGateway(
            mMessage->senderUUID);
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
    return resultDone();
}

const string ReceiveResultMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveResultMaxFlowCalculationTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}


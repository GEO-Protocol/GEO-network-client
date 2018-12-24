#include "ReceiveResultMaxFlowCalculationTransaction.h"

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    ResultMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mSenderIsGateway(false)
{}

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    ResultMaxFlowCalculationGatewayMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
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
    info() << "sender: " << mMessage->senderAddresses.at(0)->fullAddress();
    info() << "sender is gateway: " << mSenderIsGateway;
    info() << "beforeInsert mapTrustLinesCount: " << mTopologyTrustLineManager->trustLinesCounts();
    info() << "receivedTrustLinesOut: " << mMessage->outgoingFlows().size();
#endif

    auto senderID = mTopologyTrustLineManager->getID(mMessage->senderAddresses.at(0));
    if (mSenderIsGateway) {
        mTopologyTrustLineManager->addGateway(senderID);
    }
    info() << "receivedTrustLinesOut: " << mMessage->outgoingFlows().size();
    for (auto const &outgoingFlow : mMessage->outgoingFlows()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "\t" << outgoingFlow.first->fullAddress() << " " << *outgoingFlow.second.get();
#endif
        auto targetID = mTopologyTrustLineManager->getID(outgoingFlow.first);
        mTopologyTrustLineManager->addTrustLine(
            make_shared<TopologyTrustLine>(
                senderID,
                targetID,
                outgoingFlow.second));
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "receivedTrustLinesIn: " << mMessage->incomingFlows().size();
#endif
    for (auto const &incomingFlow : mMessage->incomingFlows()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "\t" << incomingFlow.first->fullAddress() << " " << *incomingFlow.second.get();
#endif
        auto sourceID = mTopologyTrustLineManager->getID(incomingFlow.first);
        mTopologyTrustLineManager->addTrustLine(
            make_shared<TopologyTrustLine>(
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


#include "ReceiveResultMaxFlowCalculationTransaction.h"

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager)
{}

ResultMaxFlowCalculationMessage::Shared ReceiveResultMaxFlowCalculationTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "initiator: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "beforeInsert mapTrustLinesCount: " << mMaxFlowCalculationTrustLineManager->trustLinesCounts();
    info() << "run\t" << "receivedTrustLinesOut: " << mMessage->outgoingFlows().size();
#endif
    for (auto const &outgoingFlow : mMessage->outgoingFlows()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "run\t" << outgoingFlow.first << " " << *outgoingFlow.second.get();
#endif
        mMaxFlowCalculationTrustLineManager->addTrustLine(
            make_shared<MaxFlowCalculationTrustLine>(
                mMessage->senderUUID,
                outgoingFlow.first,
                outgoingFlow.second));
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "receivedTrustLinesIn: " << mMessage->incomingFlows().size();
#endif
    for (auto const &incomingFlow : mMessage->incomingFlows()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "run\t" << incomingFlow.first << " " << *incomingFlow.second.get();
#endif
        mMaxFlowCalculationTrustLineManager->addTrustLine(
            make_shared<MaxFlowCalculationTrustLine>(
                incomingFlow.first,
                mMessage->senderUUID,
                incomingFlow.second));
    }

#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "afterInsert mapTrustLinesCount: " << mMaxFlowCalculationTrustLineManager->trustLinesCounts();
    mMaxFlowCalculationTrustLineManager->printTrustLines();
#endif
    return resultDone();
}

const string ReceiveResultMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveResultMaxFlowCalculationTA: " << currentTransactionUUID() << "]";
    return s.str();
}


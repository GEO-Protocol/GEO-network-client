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
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager) {}

ResultMaxFlowCalculationMessage::Shared ReceiveResultMaxFlowCalculationTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationTransaction::run() {

    info() << "run\t" << "initiator: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID();

    info() << "run\t" << "beforeInsert mapTrustLinesCount: " << mMaxFlowCalculationTrustLineManager->trustLinesCounts();

    info() << "run\t" << "receivedTrustLinesOut: " << mMessage->outgoingFlows().size();
    for (auto const &outgoingFlow : mMessage->outgoingFlows()) {
        info() << "run\t" << outgoingFlow.first << " " << outgoingFlow.second;


        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            mMessage->senderUUID(),
            outgoingFlow.first,
            outgoingFlow.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
    }
    info() << "run\t" << "receivedTrustLinesIn: " << mMessage->incomingFlows().size();
    for (auto const &incomingFlow : mMessage->incomingFlows()) {
        info() << "run\t" << incomingFlow.first << " " << incomingFlow.second;

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            incomingFlow.first,
            mMessage->senderUUID(),
            incomingFlow.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
    }

    info() << "run\t" << "afterInsert mapTrustLinesCount: " << mMaxFlowCalculationTrustLineManager->trustLinesCounts();

#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    mMaxFlowCalculationTrustLineManager->printTrustLines();
#endif

    return make_shared<const TransactionResult>(
        TransactionState::exit());
}

const string ReceiveResultMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveResultMaxFlowCalculationTA]";

    return s.str();
}


#include "MaxFlowCalculationSourceFstLevelTransaction.h"

MaxFlowCalculationSourceFstLevelTransaction::MaxFlowCalculationSourceFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceFstLevelMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceFstLevelTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(trustLinesManager) {}

MaxFlowCalculationSourceFstLevelMessage::Shared MaxFlowCalculationSourceFstLevelTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationSourceFstLevelTransaction::run() {

#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();
#endif

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
        if (nodeUUIDOutgoingFlow == mMessage->senderUUID) {
            continue;
        }
#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
        info() << "sendFirst\t" << nodeUUIDOutgoingFlow;
#endif

        sendMessage<MaxFlowCalculationSourceSndLevelMessage>(
                nodeUUIDOutgoingFlow,
                mNodeUUID,
                mMessage->senderUUID);
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

const string MaxFlowCalculationSourceFstLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationSourceFstLevelTA]";

    return s.str();
}

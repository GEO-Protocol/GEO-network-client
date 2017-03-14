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

    info() << "run\t" << "Iam: " << mNodeUUID.stringUUID();
    info() << "run\t" << "sender: " << mMessage->senderUUID().stringUUID();
    info() << "run\t" << "target: " << mMessage->targetUUID().stringUUID();
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
        if (nodeUUIDOutgoingFlow == mMessage->targetUUID() || nodeUUIDOutgoingFlow == mMessage->senderUUID()) {
            continue;
        }

        info() << "sendFirst\t" << nodeUUIDOutgoingFlow.stringUUID();

        sendMessage<MaxFlowCalculationSourceSndLevelMessage>(
                nodeUUIDOutgoingFlow,
                mNodeUUID,
                mMessage->targetUUID());
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

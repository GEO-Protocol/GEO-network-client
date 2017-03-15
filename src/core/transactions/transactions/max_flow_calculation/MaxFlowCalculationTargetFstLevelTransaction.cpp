#include "MaxFlowCalculationTargetFstLevelTransaction.h"

MaxFlowCalculationTargetFstLevelTransaction::MaxFlowCalculationTargetFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetFstLevelMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetFstLevelTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager) {}

MaxFlowCalculationTargetFstLevelMessage::Shared MaxFlowCalculationTargetFstLevelTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationTargetFstLevelTransaction::run() {

    info() << "run\t" << "Iam: " << mNodeUUID.stringUUID();
    info() << "run\t" << "sender: " << mMessage->senderUUID().stringUUID();
    info() << "run\t" << "target: " << mMessage->targetUUID().stringUUID();
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();

    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        if (nodeUUIDIncomingFlow == mMessage->senderUUID() || nodeUUIDIncomingFlow == mMessage->targetUUID()) {
            continue;
        }

        info() << "sendFirst\t" << nodeUUIDIncomingFlow.stringUUID();

        sendMessage<MaxFlowCalculationTargetSndLevelMessage>(
                nodeUUIDIncomingFlow,
                mNodeUUID,
                mMessage->targetUUID());
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

const string MaxFlowCalculationTargetFstLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTargetFstLevelTA]";

    return s.str();
}

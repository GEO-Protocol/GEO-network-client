#include "MaxFlowCalculationTargetFstLevelTransaction.h"

MaxFlowCalculationTargetFstLevelTransaction::MaxFlowCalculationTargetFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetFstLevelInMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetFstLevelTransactionType,
        nodeUUID
    ),
    mMessage(message),
    mTrustLinesManager(manager),
    mLog(logger){}

MaxFlowCalculationTargetFstLevelInMessage::Shared MaxFlowCalculationTargetFstLevelTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationTargetFstLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->getFirstLevelNeighborsWithIncomingFlow();
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        if (nodeUUIDIncomingFlow == mMessage->senderUUID() || nodeUUIDIncomingFlow == mMessage->targetUUID()) {
            continue;
        }
        Message *message = new MaxFlowCalculationTargetFstLevelOutMessage(
            mNodeUUID,
            mMessage->targetUUID());

        mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->sendFirst", ((NodeUUID)nodeUUIDIncomingFlow).stringUUID());
        addMessage(
            Message::Shared(message),
            nodeUUIDIncomingFlow);
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

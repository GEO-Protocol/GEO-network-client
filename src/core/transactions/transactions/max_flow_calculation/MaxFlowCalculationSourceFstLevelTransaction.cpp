#include "MaxFlowCalculationSourceFstLevelTransaction.h"

MaxFlowCalculationSourceFstLevelTransaction::MaxFlowCalculationSourceFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceFstLevelInMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceFstLevelTransactionType,
        nodeUUID
    ),
    mMessage(message),
    mTrustLinesManager(trustLinesManager),
    mLog(logger){}

MaxFlowCalculationSourceFstLevelInMessage::Shared MaxFlowCalculationSourceFstLevelTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationSourceFstLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->outgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->incomingFlows().size()));

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
        if (nodeUUIDOutgoingFlow == mMessage->targetUUID() || nodeUUIDOutgoingFlow == mMessage->senderUUID()) {
            continue;
        }
        Message *message = new MaxFlowCalculationSourceFstLevelOutMessage(
            mNodeUUID,
            mMessage->targetUUID());

        mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->sendFirst",
                      ((NodeUUID)nodeUUIDOutgoingFlow).stringUUID());
        addMessage(
            Message::Shared(message),
            nodeUUIDOutgoingFlow);
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

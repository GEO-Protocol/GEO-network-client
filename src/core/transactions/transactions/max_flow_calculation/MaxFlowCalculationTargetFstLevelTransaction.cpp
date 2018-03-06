#include "MaxFlowCalculationTargetFstLevelTransaction.h"

MaxFlowCalculationTargetFstLevelTransaction::MaxFlowCalculationTargetFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetFstLevelMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetFstLevelTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mIAmGateway(iAmGateway)
{}

MaxFlowCalculationTargetFstLevelMessage::Shared MaxFlowCalculationTargetFstLevelTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationTargetFstLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "target: " << mMessage->targetUUID();
    info() << "run\t" << "i am is gateway: " << mIAmGateway;
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();
#endif
    vector<NodeUUID> incomingFlowUuids;
    if (mIAmGateway) {
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
        // inform that I am is gateway
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
            mMessage->targetUUID(),
            mEquivalent,
            mNodeUUID,
            outgoingFlows,
            incomingFlows);
        incomingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
    } else {
        incomingFlowUuids = mTrustLinesManager->firstLevelNonGatewayNeighborsWithIncomingFlow();
    }
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        if (nodeUUIDIncomingFlow == mMessage->senderUUID || nodeUUIDIncomingFlow == mMessage->targetUUID()) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeUUIDIncomingFlow;
#endif
        sendMessage<MaxFlowCalculationTargetSndLevelMessage>(
            nodeUUIDIncomingFlow,
            mEquivalent,
            mNodeUUID,
            mMessage->targetUUID());
    }
    return resultDone();
}

const string MaxFlowCalculationTargetFstLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTargetFstLevelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

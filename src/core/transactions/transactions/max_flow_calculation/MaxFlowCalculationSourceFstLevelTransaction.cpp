#include "MaxFlowCalculationSourceFstLevelTransaction.h"

MaxFlowCalculationSourceFstLevelTransaction::MaxFlowCalculationSourceFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceFstLevelMessage::Shared message,
    TrustLinesManager *trustLinesManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceFstLevelTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLinesManager(trustLinesManager),
    mIAmGateway(iAmGateway)
{}

MaxFlowCalculationSourceFstLevelMessage::Shared MaxFlowCalculationSourceFstLevelTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst MaxFlowCalculationSourceFstLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "i am is gateway: " << mIAmGateway;
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();
#endif
    vector<NodeUUID> outgoingFlowUuids;
    if (mIAmGateway) {
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
        // inform that I am is gateway
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            outgoingFlows,
            incomingFlows);
        outgoingFlowUuids = mTrustLinesManager->firstLevelGatewayNeighborsWithOutgoingFlow();
    } else {
        outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    }
    for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
        if (nodeUUIDOutgoingFlow == mMessage->senderUUID) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeUUIDOutgoingFlow;
#endif
        sendMessage<MaxFlowCalculationSourceSndLevelMessage>(
            nodeUUIDOutgoingFlow,
            mEquivalent,
            mNodeUUID,
            mMessage->senderUUID);
    }
    return resultDone();
}

const string MaxFlowCalculationSourceFstLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationSourceFstLevelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

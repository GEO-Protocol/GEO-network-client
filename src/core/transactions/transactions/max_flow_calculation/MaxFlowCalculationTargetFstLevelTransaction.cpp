#include "MaxFlowCalculationTargetFstLevelTransaction.h"

MaxFlowCalculationTargetFstLevelTransaction::MaxFlowCalculationTargetFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetFstLevelMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetFstLevelTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorsManager (contractorsManager),
    mTrustLinesManager(manager),
    mIAmGateway(iAmGateway)
{}

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
    vector<ContractorID> incomingFlowIDs;
    if (mIAmGateway) {
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
        vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsNew;
        vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsNew;
        // inform that I am is gateway
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
            mMessage->targetAddresses().at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            outgoingFlows,
            incomingFlows);
        if (mMessage->isTargetGateway()) {
            incomingFlowUuids = mTrustLinesManager->firstLevelGatewayNeighborsWithIncomingFlow();
        } else {
            incomingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithIncomingFlow();
        }
    } else {
        incomingFlowIDs = mTrustLinesManager->firstLevelNonGatewayNeighborsWithIncomingFlow();
    }
    auto targetContractorID = mContractorsManager->contractorIDByAddress(mMessage->targetAddresses().at(0));
    for (auto const &nodeIDWithIncomingFlow : incomingFlowIDs) {
        if (nodeIDWithIncomingFlow == mMessage->idOnReceiverSide ||
                nodeIDWithIncomingFlow == targetContractorID) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeIDWithIncomingFlow;
#endif
        sendMessage<MaxFlowCalculationTargetSndLevelMessage>(
            nodeIDWithIncomingFlow,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->idOnContractorSide(nodeIDWithIncomingFlow),
            mMessage->targetUUID(),
            mMessage->targetAddresses());
            mMessage->targetUUID(),
            mMessage->isTargetGateway());
    }
    return resultDone();
}

const string MaxFlowCalculationTargetFstLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationTargetFstLevelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

#include "MaxFlowCalculationSourceFstLevelTransaction.h"

MaxFlowCalculationSourceFstLevelTransaction::MaxFlowCalculationSourceFstLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationSourceFstLevelMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    Logger &logger,
    bool iAmGateway) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceFstLevelTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorsManager (contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst MaxFlowCalculationSourceFstLevelTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "Iam: " << mNodeUUID;
    info() << "run\t" << "sender: " << mMessage->senderUUID;
    info() << "run\t" << "i am is gateway: " << mIAmGateway;
    info() << "run\t" << "OutgoingFlows: " << mTrustLinesManager->outgoingFlows().size();
    info() << "run\t" << "IncomingFlows: " << mTrustLinesManager->incomingFlows().size();
#endif
    vector<ContractorID> outgoingFlowIDs;
    if (mIAmGateway) {
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows;
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows;
        vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsNew;
        vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsNew;
        // inform that I am is gateway
        // todo : it is not required inform about gateway, because this info initiator can obtain on it side
        auto contractorsAddresses = mContractorsManager->contractorAddresses(mMessage->idOnReceiverSide);
        sendMessage<ResultMaxFlowCalculationGatewayMessage>(
            contractorsAddresses.at(0),
            mEquivalent,
            mNodeUUID,
            mContractorsManager->ownAddresses(),
            outgoingFlows,
            incomingFlows,
            outgoingFlowsNew,
            incomingFlowsNew);
        outgoingFlowIDs = mTrustLinesManager->firstLevelGatewayNeighborsWithOutgoingFlow();
    } else {
        outgoingFlowIDs = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    }
    for (auto const &nodeIDWithOutgoingFlow : outgoingFlowIDs) {
        if (nodeIDWithOutgoingFlow == mMessage->idOnReceiverSide) {
            continue;
        }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeIDWithOutgoingFlow;
#endif
        sendMessage<MaxFlowCalculationSourceSndLevelMessage>(
            nodeIDWithOutgoingFlow,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->idOnContractorSide(nodeIDWithOutgoingFlow),
            mMessage->senderUUID,
            mContractorsManager->contractorAddresses(mMessage->idOnReceiverSide));
    }
    return resultDone();
}

const string MaxFlowCalculationSourceFstLevelTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationSourceFstLevelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

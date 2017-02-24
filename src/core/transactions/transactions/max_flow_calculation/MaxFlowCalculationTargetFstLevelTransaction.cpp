//
// Created by mc on 16.02.17.
//

#include "MaxFlowCalculationTargetFstLevelTransaction.h"

MaxFlowCalculationTargetFstLevelTransaction::MaxFlowCalculationTargetFstLevelTransaction(
    NodeUUID &nodeUUID,
    MaxFlowCalculationTargetFstLevelInMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetFstLevelTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mLog(logger){}

MaxFlowCalculationTargetFstLevelTransaction::MaxFlowCalculationTargetFstLevelTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

MaxFlowCalculationTargetFstLevelInMessage::Shared MaxFlowCalculationTargetFstLevelTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> MaxFlowCalculationTargetFstLevelTransaction::serializeToBytes() const {

    auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    auto messageBytesAndCount = mMessage->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second + messageBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        messageBytesAndCount.first.get(),
        messageBytesAndCount.second);
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void MaxFlowCalculationTargetFstLevelTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(MaxFlowCalculationTargetFstLevelInMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
        MaxFlowCalculationTargetFstLevelInMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    mMessage = MaxFlowCalculationTargetFstLevelInMessage::Shared(
        new MaxFlowCalculationTargetFstLevelInMessage(
            messageBufferShared));
}

TransactionResult::SharedConst MaxFlowCalculationTargetFstLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    //mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->getFirstLevelNeighborsWithIncomingFlow();
    NodeUUID targetUUID = mMessage->targetUUID();
    for (auto const &it : incomingFlowUuids) {
        Message *message = new MaxFlowCalculationTargetFstLevelOutMessage(
            targetUUID);

        mLog->logInfo("MaxFlowCalculationTargetFstLevelTransaction->sendFirst", ((NodeUUID)it).stringUUID());
        addMessage(
            Message::Shared(message),
            it);
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

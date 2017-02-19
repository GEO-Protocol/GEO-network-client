//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationTargetSndLevelTransaction.h"

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    NodeUUID &nodeUUID,
    MaxFlowCalculationTargetSndLevelInMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    MaxFlowCalculationTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetSndLevelTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mLog(logger){}

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

MaxFlowCalculationTargetSndLevelInMessage::Shared MaxFlowCalculationTargetSndLevelTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> MaxFlowCalculationTargetSndLevelTransaction::serializeToBytes() const {

    auto parentBytesAndCount = MaxFlowCalculationTransaction::serializeToBytes();
    auto messageBytesAndCount = mMessage->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second + messageBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        messageBytesAndCount.first.get(),
        messageBytesAndCount.second
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void MaxFlowCalculationTargetSndLevelTransaction::deserializeFromBytes(
    BytesShared buffer) {

    MaxFlowCalculationTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(MaxFlowCalculationTargetSndLevelInMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes(),
        MaxFlowCalculationTargetSndLevelInMessage::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mMessage = MaxFlowCalculationTargetSndLevelInMessage::Shared(
        new MaxFlowCalculationTargetSndLevelInMessage(
            messageBufferShared
        )
    );
}

TransactionResult::SharedConst MaxFlowCalculationTargetSndLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());

    sendResultToInitiator();

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

void MaxFlowCalculationTargetSndLevelTransaction::sendResultToInitiator() {

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResult",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResult",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    map<NodeUUID, TrustLineAmount> incomingFlows = mTrustLinesManager->getIncomingFlows();
    Message *message = new SendResultMaxFlowCalculationFromTargetMessage(
        mNodeUUID,
        mNodeUUID,
        mTransactionUUID,
        incomingFlows
    );

    addMessage(
        Message::Shared(message),
        mMessage->targetUUID()
    );
}

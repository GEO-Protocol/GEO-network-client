//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationTargetSndLevelTransaction.h"

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    const NodeUUID &nodeUUID,
    MaxFlowCalculationTargetSndLevelInMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetSndLevelTransactionType,
        nodeUUID
    ),
    mMessage(message),
    mTrustLinesManager(manager),
    mLog(logger){}

MaxFlowCalculationTargetSndLevelTransaction::MaxFlowCalculationTargetSndLevelTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    MaxFlowCalculationTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationTargetSndLevelTransactionType
    ),
    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

MaxFlowCalculationTargetSndLevelInMessage::Shared MaxFlowCalculationTargetSndLevelTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> MaxFlowCalculationTargetSndLevelTransaction::serializeToBytes() const {

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

void MaxFlowCalculationTargetSndLevelTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(MaxFlowCalculationTargetSndLevelInMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
        MaxFlowCalculationTargetSndLevelInMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    mMessage = MaxFlowCalculationTargetSndLevelInMessage::Shared(
        new MaxFlowCalculationTargetSndLevelInMessage(
            messageBufferShared));
}

TransactionResult::SharedConst MaxFlowCalculationTargetSndLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    //mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());

    sendResultToInitiator();

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

void MaxFlowCalculationTargetSndLevelTransaction::sendResultToInitiator() {

    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResult",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationTargetSndLevelTransaction->sendResult",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    map<NodeUUID, TrustLineAmount> outgoingFlows;
    for (auto const &outgoingFlow : mTrustLinesManager->getOutgoingFlows()) {
        if (outgoingFlow.first == mMessage->senderUUID()) {
            outgoingFlows.insert(outgoingFlow);
        }
    }
    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->getIncomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()) {
            incomingFlows.insert(incomingFlow);
        }
    }

    for (auto const &it : incomingFlows) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction::sendResult", it.first.stringUUID());
    }

    Message *message = new SendResultMaxFlowCalculationMessage(
        mNodeUUID,
        outgoingFlows,
        incomingFlows);

    addMessage(
        Message::Shared(message),
        mMessage->targetUUID());
}

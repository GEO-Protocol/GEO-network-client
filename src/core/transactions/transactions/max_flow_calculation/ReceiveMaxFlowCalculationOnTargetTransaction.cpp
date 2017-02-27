//
// Created by mc on 15.02.17.
//

#include "ReceiveMaxFlowCalculationOnTargetTransaction.h"

ReceiveMaxFlowCalculationOnTargetTransaction::ReceiveMaxFlowCalculationOnTargetTransaction(
        NodeUUID &nodeUUID,
        ReceiveMaxFlowCalculationOnTargetMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger) :

        BaseTransaction(
                BaseTransaction::TransactionType::ReceiveMaxFlowCalculationOnTargetTransactionType,
                nodeUUID),
        mMessage(message),
        mTrustLinesManager(manager),
        mLog(logger){}

ReceiveMaxFlowCalculationOnTargetTransaction::ReceiveMaxFlowCalculationOnTargetTransaction(
        BytesShared buffer,
        TrustLinesManager *manager) :

        mTrustLinesManager(manager){

        deserializeFromBytes(buffer);
}

ReceiveMaxFlowCalculationOnTargetMessage::Shared ReceiveMaxFlowCalculationOnTargetTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> ReceiveMaxFlowCalculationOnTargetTransaction::serializeToBytes() const {

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

void ReceiveMaxFlowCalculationOnTargetTransaction::deserializeFromBytes(
        BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(ReceiveMaxFlowCalculationOnTargetMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
        ReceiveMaxFlowCalculationOnTargetMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    mMessage = ReceiveMaxFlowCalculationOnTargetMessage::Shared(
        new ReceiveMaxFlowCalculationOnTargetMessage(
            messageBufferShared));
}

TransactionResult::SharedConst ReceiveMaxFlowCalculationOnTargetTransaction::run() {

    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run", "target: " + mNodeUUID.stringUUID());
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run", "initiator: " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("ReceiveMaxFlowCalculationTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    sendResultToInitiator();
    sendMessagesOnFirstLevel();
    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendResultToInitiator() {

    map<NodeUUID, TrustLineAmount> outgoingFlows;
    map<NodeUUID, TrustLineAmount> incomingFlows;
    for (auto const &incomingFlow : mTrustLinesManager->getIncomingFlows()) {
        if (incomingFlow.first != mMessage->senderUUID()) {
            incomingFlows.insert(incomingFlow);
        }
    }

    Message *message = new SendResultMaxFlowCalculationMessage(
        mNodeUUID,
        outgoingFlows,
        incomingFlows);

    addMessage(
        Message::Shared(message),
        mMessage->senderUUID()
    );
    mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendResultToInitiator",
                  "send to " + mMessage->senderUUID().stringUUID());
}

void ReceiveMaxFlowCalculationOnTargetTransaction::sendMessagesOnFirstLevel() {

    vector<NodeUUID> incomingFlowUuids = mTrustLinesManager->getFirstLevelNeighborsWithIncomingFlow();
    for (auto const &nodeUUIDIncomingFlow : incomingFlowUuids) {
        NodeUUID targetUUID = mMessage->targetUUID();
        Message *message = new SendMaxFlowCalculationTargetFstLevelMessage(
            mNodeUUID,
            targetUUID);

        mLog->logInfo("ReceiveMaxFlowCalculationOnTargetTransaction->sendFirst",
                      ((NodeUUID)nodeUUIDIncomingFlow).stringUUID());
        addMessage(
            Message::Shared(message),
            nodeUUIDIncomingFlow);
    }

}

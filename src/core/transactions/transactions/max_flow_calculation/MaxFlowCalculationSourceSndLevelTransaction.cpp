//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationSourceSndLevelTransaction.h"

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    NodeUUID &nodeUUID,
    MaxFlowCalculationSourceSndLevelInMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    MaxFlowCalculationTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceSndLevelTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mLog(logger){}

MaxFlowCalculationSourceSndLevelTransaction::MaxFlowCalculationSourceSndLevelTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

MaxFlowCalculationSourceSndLevelInMessage::Shared MaxFlowCalculationSourceSndLevelTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> MaxFlowCalculationSourceSndLevelTransaction::serializeToBytes() const {

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

void MaxFlowCalculationSourceSndLevelTransaction::deserializeFromBytes(
    BytesShared buffer) {

    MaxFlowCalculationTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(MaxFlowCalculationSourceSndLevelInMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes(),
        MaxFlowCalculationSourceSndLevelInMessage::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mMessage = MaxFlowCalculationSourceSndLevelInMessage::Shared(
        new MaxFlowCalculationSourceSndLevelInMessage(
            messageBufferShared
        )
    );
}

TransactionResult::SharedConst MaxFlowCalculationSourceSndLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());

    sendResultToInitiator();

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

void MaxFlowCalculationSourceSndLevelTransaction::sendResultToInitiator() {

    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResultToInitiator",
                  "send to " + mMessage->targetUUID().stringUUID());
    map<NodeUUID, TrustLineAmount> outgoingFlows = mTrustLinesManager->getOutgoingFlows();
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResult",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction->sendResult",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    for (auto const &it : outgoingFlows) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("MaxFlowCalculationSourceSndLevelTransaction::sendResult", it.first.stringUUID());
    }

    Message *message = new SendResultMaxFlowCalculationFromSourceMessage(
        mNodeUUID,
        mTransactionUUID,
        outgoingFlows
    );

    addMessage(
        Message::Shared(message),
        mMessage->targetUUID()
    );
}
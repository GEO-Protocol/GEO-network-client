//
// Created by mc on 15.02.17.
//

#include "ReceiveResultMaxFlowCalculationFromTargetTransaction.h"

ReceiveResultMaxFlowCalculationFromTargetTransaction::ReceiveResultMaxFlowCalculationFromTargetTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationFromTargetMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    MaxFlowCalculationTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationFromTargetTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mLog(logger){}

ReceiveResultMaxFlowCalculationFromTargetTransaction::ReceiveResultMaxFlowCalculationFromTargetTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

ResultMaxFlowCalculationFromTargetMessage::Shared ReceiveResultMaxFlowCalculationFromTargetTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> ReceiveResultMaxFlowCalculationFromTargetTransaction::serializeToBytes() const {

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

void ReceiveResultMaxFlowCalculationFromTargetTransaction::deserializeFromBytes(
    BytesShared buffer) {

    MaxFlowCalculationTransaction::deserializeFromBytes(buffer);
    //BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize());
    BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize(
        buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes()));
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes(),
        //ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize()
        ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize(
            buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes())
    );
    //-----------------------------------------------------
    mMessage = ResultMaxFlowCalculationFromTargetMessage::Shared(
        new ResultMaxFlowCalculationFromTargetMessage(
            messageBufferShared
        )
    );
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationFromTargetTransaction::run() {

    mLog->logInfo("ReceiveResultMaxFlowCalculationFromTargetTransaction->run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("ReceiveResultMaxFlowCalculationFromTargetTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());

    mLog->logInfo("ReceiveResultMaxFlowCalculationFromTargetTransaction::run",
                  "received trustLines: " + to_string(mMessage->getIncomingFlows().size()));
    for (auto const &it : mMessage->getIncomingFlows()) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("ReceiveResultMaxFlowCalculationFromTargetTransaction::run",
                      it.first.stringUUID() + " " + to_string((uint32_t)trustLineAmount));
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

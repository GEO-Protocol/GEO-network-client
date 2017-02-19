//
// Created by mc on 17.02.17.
//

#include "ReceiveResultMaxFlowCalculationFromSourceTransaction.h"

ReceiveResultMaxFlowCalculationFromSourceTransaction::ReceiveResultMaxFlowCalculationFromSourceTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationFromSourceMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    MaxFlowCalculationTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationFromSourceTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mLog(logger){}

ReceiveResultMaxFlowCalculationFromSourceTransaction::ReceiveResultMaxFlowCalculationFromSourceTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

ResultMaxFlowCalculationFromSourceMessage::Shared ReceiveResultMaxFlowCalculationFromSourceTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> ReceiveResultMaxFlowCalculationFromSourceTransaction::serializeToBytes() const {

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

void ReceiveResultMaxFlowCalculationFromSourceTransaction::deserializeFromBytes(
    BytesShared buffer) {

    MaxFlowCalculationTransaction::deserializeFromBytes(buffer);
    //BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationFromSourceMessage::kRequestedBufferSize());
    BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationFromSourceMessage::kRequestedBufferSize(
        buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes()));
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes(),
        //ResultMaxFlowCalculationFromSourceMessage::kRequestedBufferSize()
        ResultMaxFlowCalculationFromSourceMessage::kRequestedBufferSize(
            buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes())
    );
    //-----------------------------------------------------
    mMessage = ResultMaxFlowCalculationFromSourceMessage::Shared(
        new ResultMaxFlowCalculationFromSourceMessage(
            messageBufferShared
        )
    );
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationFromSourceTransaction::run() {

    mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction->run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());

    mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
                  "received trustLines: " + to_string(mMessage->getOutgoingFlows().size()));
    for (auto const &it : mMessage->getOutgoingFlows()) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
                      it.first.stringUUID() + " " + to_string((uint32_t)trustLineAmount));
    }
    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

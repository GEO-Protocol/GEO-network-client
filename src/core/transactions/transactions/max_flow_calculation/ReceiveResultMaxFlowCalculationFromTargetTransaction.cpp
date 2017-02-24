//
// Created by mc on 15.02.17.
//

#include "ReceiveResultMaxFlowCalculationFromTargetTransaction.h"

ReceiveResultMaxFlowCalculationFromTargetTransaction::ReceiveResultMaxFlowCalculationFromTargetTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationFromTargetMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationFromTargetTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mLog(logger){}

ReceiveResultMaxFlowCalculationFromTargetTransaction::ReceiveResultMaxFlowCalculationFromTargetTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager) :

    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager){

    deserializeFromBytes(buffer);
}

ResultMaxFlowCalculationFromTargetMessage::Shared ReceiveResultMaxFlowCalculationFromTargetTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> ReceiveResultMaxFlowCalculationFromTargetTransaction::serializeToBytes() const {

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

void ReceiveResultMaxFlowCalculationFromTargetTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    //BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize());
    BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize(
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes()));
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
        //ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize()
        ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize(
            buffer.get() + BaseTransaction::kOffsetToInheritedBytes()));
    //-----------------------------------------------------
    mMessage = ResultMaxFlowCalculationFromTargetMessage::Shared(
        new ResultMaxFlowCalculationFromTargetMessage(
            messageBufferShared));
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

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            it.first,
            mMessage->senderUUID(),
            it.second
        );

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
        mMaxFlowCalculationTrustLineManager->addFlow(it.first, mMessage->senderUUID(),  it.second);
    }

    mLog->logInfo("ReceiveResultMaxFlowCalculationFromTargetTransaction::run",
                  "trustLineMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mvTrustLines.size()));
    for (const auto &it : mMaxFlowCalculationTrustLineManager->mvTrustLines) {
        mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
                      "key: " + ((NodeUUID)it.first).stringUUID());
        for (const auto &it1 : it.second) {
            MaxFlowCalculationTrustLine::Shared trustLine = it1;
            mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
                          "value: " + trustLine->getTargetUUID().stringUUID() + " "
                          + to_string((uint32_t)trustLine->getAmount()));
        }
    }

    mLog->logInfo("ReceiveResultMaxFlowCalculationFromTargetTransaction::run",
                  "entityMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mEntities.size()));
    for (const auto &it : mMaxFlowCalculationTrustLineManager->mEntities) {
        mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
                      "key: " + ((NodeUUID)it.first).stringUUID());

        for (const auto &it1 : it.second->mIncomingFlows) {
            mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
                          "inflow: " + ((NodeUUID) it1.first).stringUUID() + " "
                          + to_string((uint32_t)it1.second));
        }

        for (const auto &it1 : it.second->mOutgoingFlows) {
            mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
                          "outflow: " + ((NodeUUID) it1.first).stringUUID() + " "
                          + to_string((uint32_t)it1.second));
        }
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

//
// Created by mc on 17.02.17.
//

#include "ReceiveResultMaxFlowCalculationFromSourceTransaction.h"

ReceiveResultMaxFlowCalculationFromSourceTransaction::ReceiveResultMaxFlowCalculationFromSourceTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationFromSourceMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationFromSourceTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mLog(logger){}

ReceiveResultMaxFlowCalculationFromSourceTransaction::ReceiveResultMaxFlowCalculationFromSourceTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager) :

    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager){

    deserializeFromBytes(buffer);
}

ResultMaxFlowCalculationFromSourceMessage::Shared ReceiveResultMaxFlowCalculationFromSourceTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> ReceiveResultMaxFlowCalculationFromSourceTransaction::serializeToBytes() const {

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

void ReceiveResultMaxFlowCalculationFromSourceTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    // todo продумати архітектуру серіалізації - десеріалізаціїї меседжів змінної довжини
    BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationFromSourceMessage::kRequestedBufferSize(
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes()));
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
        ResultMaxFlowCalculationFromSourceMessage::kRequestedBufferSize(
            buffer.get() + BaseTransaction::kOffsetToInheritedBytes()));
    //-----------------------------------------------------
    mMessage = ResultMaxFlowCalculationFromSourceMessage::Shared(
        new ResultMaxFlowCalculationFromSourceMessage(
            messageBufferShared));
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

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            mMessage->senderUUID(),
            it.first,
            it.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
        mMaxFlowCalculationTrustLineManager->addFlow(mMessage->senderUUID(), it.first, it.second);
    }

    mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
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

    mLog->logInfo("ReceiveResultMaxFlowCalculationFromSourceTransaction::run",
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

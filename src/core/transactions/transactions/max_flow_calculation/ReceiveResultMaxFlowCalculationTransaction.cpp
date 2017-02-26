//
// Created by mc on 17.02.17.
//

#include "ReceiveResultMaxFlowCalculationTransaction.h"

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mLog(logger){}

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager) :

    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager){

    deserializeFromBytes(buffer);
}

ResultMaxFlowCalculationMessage::Shared ReceiveResultMaxFlowCalculationTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> ReceiveResultMaxFlowCalculationTransaction::serializeToBytes() const {

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

void ReceiveResultMaxFlowCalculationTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    // todo продумати архітектуру серіалізації - десеріалізаціїї меседжів змінної довжини
    BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationMessage::kRequestedBufferSize(
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes()));
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
        ResultMaxFlowCalculationMessage::kRequestedBufferSize(
            buffer.get() + BaseTransaction::kOffsetToInheritedBytes()));
    //-----------------------------------------------------
    mMessage = ResultMaxFlowCalculationMessage::Shared(
        new ResultMaxFlowCalculationMessage(
            messageBufferShared));
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationTransaction::run() {

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction->run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "received trustLines out: " + to_string(mMessage->outgoingFlows().size()));
    for (auto const &it : mMessage->outgoingFlows()) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      it.first.stringUUID() + " " + to_string((uint32_t)trustLineAmount));

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            mMessage->senderUUID(),
            it.first,
            it.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
        //mMaxFlowCalculationTrustLineManager->addFlow(mMessage->senderUUID(), it.first, it.second);
    }
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "received trustLines in: " + to_string(mMessage->incomingFlows().size()));
    for (auto const &it : mMessage->incomingFlows()) {
        TrustLineAmount trustLineAmount = it.second;
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      it.first.stringUUID() + " " + to_string((uint32_t)trustLineAmount));

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            it.first,
            mMessage->senderUUID(),
            it.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
        //mMaxFlowCalculationTrustLineManager->addFlow(mMessage->senderUUID(), it.first, it.second);
    }

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "trustLineMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mvTrustLines.size()));
    for (const auto &it : mMaxFlowCalculationTrustLineManager->mvTrustLines) {
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      "key: " + ((NodeUUID)it.first).stringUUID());
        for (const auto &it1 : it.second) {
            MaxFlowCalculationTrustLine::Shared trustLine = it1;
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "value: " + trustLine->getTargetUUID().stringUUID() + " "
                          + to_string((uint32_t)trustLine->getAmount()));
        }
    }

    /*mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "entityMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mEntities.size()));
    for (const auto &it : mMaxFlowCalculationTrustLineManager->mEntities) {
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      "key: " + ((NodeUUID)it.first).stringUUID());

        for (const auto &it1 : it.second->mIncomingFlows) {
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "inflow: " + ((NodeUUID) it1.first).stringUUID() + " "
                          + to_string((uint32_t)it1.second));
        }

        for (const auto &it1 : it.second->mOutgoingFlows) {
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "outflow: " + ((NodeUUID) it1.first).stringUUID() + " "
                          + to_string((uint32_t)it1.second));
        }
    }*/

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

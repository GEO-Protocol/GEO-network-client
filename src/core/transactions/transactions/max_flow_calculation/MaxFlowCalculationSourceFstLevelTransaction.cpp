//
// Created by mc on 16.02.17.
//

#include "MaxFlowCalculationSourceFstLevelTransaction.h"

MaxFlowCalculationSourceFstLevelTransaction::MaxFlowCalculationSourceFstLevelTransaction(
    NodeUUID &nodeUUID,
    MaxFlowCalculationSourceFstLevelInMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    MaxFlowCalculationTransaction(
        BaseTransaction::TransactionType::MaxFlowCalculationSourceFstLevelTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mLog(logger){}

MaxFlowCalculationSourceFstLevelTransaction::MaxFlowCalculationSourceFstLevelTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

MaxFlowCalculationSourceFstLevelInMessage::Shared MaxFlowCalculationSourceFstLevelTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> MaxFlowCalculationSourceFstLevelTransaction::serializeToBytes() const {

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

void MaxFlowCalculationSourceFstLevelTransaction::deserializeFromBytes(
    BytesShared buffer) {

    MaxFlowCalculationTransaction::deserializeFromBytes(buffer);
    BytesShared messageBufferShared = tryCalloc(MaxFlowCalculationSourceFstLevelInMessage::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes(),
        MaxFlowCalculationSourceFstLevelInMessage::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mMessage = MaxFlowCalculationSourceFstLevelInMessage::Shared(
        new MaxFlowCalculationSourceFstLevelInMessage(
            messageBufferShared
        )
    );
}

TransactionResult::SharedConst MaxFlowCalculationSourceFstLevelTransaction::run() {

    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run", "Iam: " + mNodeUUID.stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run", "target: " + mMessage->targetUUID().stringUUID());
    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->getFirstLevelNeighborsWithOutgoingFlow();
    NodeUUID targetUUID = mMessage->targetUUID();
    for (auto const &it : outgoingFlowUuids) {
        if (it == targetUUID) {
            continue;
        }
        Message *message = new MaxFlowCalculationSourceFstLevelOutMessage(
            mNodeUUID,
            targetUUID,
            mTransactionUUID
        );

        mLog->logInfo("MaxFlowCalculationSourceFstLevelTransaction->sendFirst", ((NodeUUID)it).stringUUID());
        addMessage(
            Message::Shared(message),
            it
        );
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

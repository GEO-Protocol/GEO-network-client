//
// Created by mc on 15.02.17.
//

#include "SendResultMaxFlowCalculationFromTargetMessage.h"

SendResultMaxFlowCalculationFromTargetMessage::SendResultMaxFlowCalculationFromTargetMessage(
    NodeUUID &senderUUID,
    NodeUUID &targetUUID,
    TransactionUUID &transactionUUID,
    map<NodeUUID, TrustLineAmount> &incomingFlows) :

    MaxFlowCalculationMessage(senderUUID, targetUUID, transactionUUID),
    mIncomingFlows(incomingFlows){};

const Message::MessageType SendResultMaxFlowCalculationFromTargetMessage::typeID() const {

    return Message::MessageTypeID::SendResultMaxFlowCalculationFromTargetMessageType;
}

pair<BytesShared, size_t> SendResultMaxFlowCalculationFromTargetMessage::serializeToBytes() {

    auto parentBytesAndCount = MaxFlowCalculationMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second + sizeof(uint32_t) +
                        /*+ mIncomingFlows.size() * */(NodeUUID::kBytesSize + kTrustLineAmountBytesCount);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    uint32_t trustLinesCount = (uint32_t)mIncomingFlows.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesCount,
        sizeof(uint32_t)
    );
    dataBytesOffset += sizeof(uint32_t);
    //----------------------------------------------------
    for (auto const &it : mIncomingFlows) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            it.first.data,
            NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        TrustLineAmount trustLineAmount = it.second;
        vector<byte> buffer = trustLineAmountToBytes(trustLineAmount);
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size()
        );
        dataBytesOffset += kTrustLineAmountBytesCount;
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void SendResultMaxFlowCalculationFromTargetMessage::deserializeFromBytes(
    BytesShared buffer){

    MaxFlowCalculationMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = MaxFlowCalculationMessage::kOffsetToInheritedBytes();
    //-----------------------------------------------------
    uint32_t *trustLinesCount = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mIncomingFlows.clear();
    for (int idx = 0; idx < *trustLinesCount; idx++) {
        NodeUUID nodeUUID;
        memcpy(
            nodeUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        //--------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //--------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mIncomingFlows.insert(make_pair(nodeUUID, trustLineAmount));
    }
}

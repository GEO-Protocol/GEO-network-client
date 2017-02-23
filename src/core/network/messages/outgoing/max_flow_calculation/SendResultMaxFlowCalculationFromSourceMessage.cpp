//
// Created by mc on 17.02.17.
//

#include "SendResultMaxFlowCalculationFromSourceMessage.h"

SendResultMaxFlowCalculationFromSourceMessage::SendResultMaxFlowCalculationFromSourceMessage(
    NodeUUID &senderUUID,
    TransactionUUID &transactionUUID,
    map<NodeUUID, TrustLineAmount> &outgoingFlows) :

    TransactionMessage(senderUUID, transactionUUID),
    mOutgoingFlows(outgoingFlows){};

const Message::MessageType SendResultMaxFlowCalculationFromSourceMessage::typeID() const {

    return Message::MessageTypeID::SendResultMaxFlowCalculationFromSourceMessageType;
}

pair<BytesShared, size_t> SendResultMaxFlowCalculationFromSourceMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second + sizeof(uint32_t) +
                        + mOutgoingFlows.size() * (NodeUUID::kBytesSize + kTrustLineAmountBytesCount);
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
    uint32_t trustLinesCount = (uint32_t)mOutgoingFlows.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesCount,
        sizeof(uint32_t)
    );
    dataBytesOffset += sizeof(uint32_t);
    //----------------------------------------------------
    for (auto const &it : mOutgoingFlows) {
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

void SendResultMaxFlowCalculationFromSourceMessage::deserializeFromBytes(
    BytesShared buffer){

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //-----------------------------------------------------
    uint32_t *trustLinesCount = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mOutgoingFlows.clear();
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
        mOutgoingFlows.insert(make_pair(nodeUUID, trustLineAmount));
    }
}

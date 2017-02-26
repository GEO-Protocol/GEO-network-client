//
// Created by mc on 17.02.17.
//

#include "ResultMaxFlowCalculationMessage.h"

ResultMaxFlowCalculationMessage::ResultMaxFlowCalculationMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ResultMaxFlowCalculationMessage::typeID() const {

    return Message::MessageTypeID::ResultMaxFlowCalculationMessageType;
}

pair<BytesShared, size_t> ResultMaxFlowCalculationMessage::serializeToBytes() {

    auto parentBytesAndCount = SenderMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(uint32_t) + mOutgoingFlows.size() * (NodeUUID::kBytesSize + kTrustLineAmountBytesCount)
                        + sizeof(uint32_t) + mIncomingFlows.size() * (NodeUUID::kBytesSize + kTrustLineAmountBytesCount);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    uint32_t trustLinesOutCount = (uint32_t)mOutgoingFlows.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesOutCount,
        sizeof(uint32_t));
    dataBytesOffset += sizeof(uint32_t);
    //----------------------------------------------------
    for (auto const &it : mOutgoingFlows) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            it.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        TrustLineAmount trustLineAmount = it.second;
        vector<byte> buffer = trustLineAmountToBytes(trustLineAmount);
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size());
        dataBytesOffset += kTrustLineAmountBytesCount;
    }
    //----------------------------------------------------
    uint32_t trustLinesInCount = (uint32_t)mOutgoingFlows.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesInCount,
        sizeof(uint32_t));
    dataBytesOffset += sizeof(uint32_t);
    //----------------------------------------------------
    for (auto const &it : mIncomingFlows) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            it.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        TrustLineAmount trustLineAmount = it.second;
        vector<byte> buffer = trustLineAmountToBytes(trustLineAmount);
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size());
        dataBytesOffset += kTrustLineAmountBytesCount;
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void ResultMaxFlowCalculationMessage::deserializeFromBytes(
    BytesShared buffer){

    SenderMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    uint32_t *trustLinesOutCount = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mOutgoingFlows.clear();
    for (int idx = 0; idx < *trustLinesOutCount; idx++) {
        NodeUUID nodeUUID;
        memcpy(
            nodeUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mOutgoingFlows.insert(make_pair(nodeUUID, trustLineAmount));
    }
    //----------------------------------------------------
    uint32_t *trustLinesInCount = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mIncomingFlows.clear();
    for (int idx = 0; idx < *trustLinesInCount; idx++) {
        NodeUUID nodeUUID;
        memcpy(
            nodeUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mIncomingFlows.insert(make_pair(nodeUUID, trustLineAmount));
    }
}

const size_t ResultMaxFlowCalculationMessage::kRequestedBufferSize() {

    static const size_t size = SenderMessage::kOffsetToInheritedBytes()
                               + sizeof(uint32_t) + NodeUUID::kBytesSize + kTrustLineAmountBytesCount;
    return size;
}

const size_t ResultMaxFlowCalculationMessage::kRequestedBufferSize(unsigned char* buffer) {

    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    uint32_t *trustLinesCount = new (buffer + bytesBufferOffset) uint32_t;
    static const size_t size = SenderMessage::kOffsetToInheritedBytes()
                               + sizeof(uint32_t) + *trustLinesCount *
                                                    (NodeUUID::kBytesSize + kTrustLineAmountBytesCount);
    return size;
}

const map<NodeUUID, TrustLineAmount> ResultMaxFlowCalculationMessage::outgoingFlows() const {
    return mOutgoingFlows;
}

const map<NodeUUID, TrustLineAmount> ResultMaxFlowCalculationMessage::incomingFlows() const {
    return mIncomingFlows;
}

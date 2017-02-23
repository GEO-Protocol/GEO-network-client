//
// Created by mc on 15.02.17.
//

#include "ResultMaxFlowCalculationFromTargetMessage.h"

ResultMaxFlowCalculationFromTargetMessage::ResultMaxFlowCalculationFromTargetMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ResultMaxFlowCalculationFromTargetMessage::typeID() const {

    return Message::MessageTypeID::ResultMaxFlowCalculationFromTargetMessageType;
}

pair<BytesShared, size_t> ResultMaxFlowCalculationFromTargetMessage::serializeToBytes() {

    auto parentBytesAndCount = MaxFlowCalculationResultMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second + sizeof(uint32_t) +
                        + mIncomingFlows.size() * (NodeUUID::kBytesSize + kTrustLineAmountBytesCount);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
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

void ResultMaxFlowCalculationFromTargetMessage::deserializeFromBytes(
    BytesShared buffer){

    MaxFlowCalculationResultMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = MaxFlowCalculationResultMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
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

const size_t ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize() {

    static const size_t size = MaxFlowCalculationResultMessage::kOffsetToInheritedBytes()
                               + sizeof(uint32_t) + NodeUUID::kBytesSize + kTrustLineAmountBytesCount;
    return size;
}

const size_t ResultMaxFlowCalculationFromTargetMessage::kRequestedBufferSize(unsigned char* buffer) {

    size_t bytesBufferOffset = MaxFlowCalculationResultMessage::kOffsetToInheritedBytes();
    uint32_t *trustLinesCount = new (buffer + bytesBufferOffset) uint32_t;
    static const size_t size = MaxFlowCalculationResultMessage::kOffsetToInheritedBytes()
                               + sizeof(uint32_t) + *trustLinesCount *
                                                    (NodeUUID::kBytesSize + kTrustLineAmountBytesCount);
    return size;
}

map<NodeUUID, TrustLineAmount> ResultMaxFlowCalculationFromTargetMessage::getIncomingFlows() {
    return mIncomingFlows;
}

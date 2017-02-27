#include "SendResultMaxFlowCalculationMessage.h"

SendResultMaxFlowCalculationMessage::SendResultMaxFlowCalculationMessage(
    const NodeUUID& senderUUID,
    map<NodeUUID, TrustLineAmount> &outgoingFlows,
    map<NodeUUID, TrustLineAmount> &incomingFlows) :

    SenderMessage(senderUUID),
    mOutgoingFlows(outgoingFlows),
    mIncomingFlows(incomingFlows){};

const Message::MessageType SendResultMaxFlowCalculationMessage::typeID() const {

    return Message::MessageTypeID::SendResultMaxFlowCalculationMessageType;
}

pair<BytesShared, size_t> SendResultMaxFlowCalculationMessage::serializeToBytes() {

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
    uint32_t trustLinesInCount = (uint32_t)mIncomingFlows.size();
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

void SendResultMaxFlowCalculationMessage::deserializeFromBytes(
    BytesShared buffer){

    SenderMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    //-----------------------------------------------------
    uint32_t *trustLinesOutCount = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mOutgoingFlows.clear();
    for (int idx = 0; idx < *trustLinesOutCount; idx++) {
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
    //-----------------------------------------------------
    uint32_t *trustLinesInCount = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mIncomingFlows.clear();
    for (int idx = 0; idx < *trustLinesInCount; idx++) {
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

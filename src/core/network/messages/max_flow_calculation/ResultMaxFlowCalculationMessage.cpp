#include "ResultMaxFlowCalculationMessage.h"

ResultMaxFlowCalculationMessage::ResultMaxFlowCalculationMessage(
        const NodeUUID& senderUUID,
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows) :

        SenderMessage(senderUUID),
        mOutgoingFlows(outgoingFlows),
        mIncomingFlows(incomingFlows){};

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
                        + sizeof(RecordCount) + mOutgoingFlows.size() * (NodeUUID::kBytesSize + kTrustLineAmountBytesCount)
                        + sizeof(RecordCount) + mIncomingFlows.size() * (NodeUUID::kBytesSize + kTrustLineAmountBytesCount);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    RecordCount trustLinesOutCount = (RecordCount)mOutgoingFlows.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesOutCount,
        sizeof(RecordCount));
    dataBytesOffset += sizeof(RecordCount);
    //----------------------------------------------------
    for (auto const &it : mOutgoingFlows) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            it.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        vector<byte> buffer = trustLineAmountToBytes(*it.second.get());
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size());
        dataBytesOffset += kTrustLineAmountBytesCount;
    }
    //----------------------------------------------------
    RecordCount trustLinesInCount = (RecordCount)mIncomingFlows.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesInCount,
        sizeof(RecordCount));
    dataBytesOffset += sizeof(RecordCount);
    //----------------------------------------------------
    for (auto const &it : mIncomingFlows) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            it.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        vector<byte> buffer = trustLineAmountToBytes(*it.second.get());
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
    RecordCount *trustLinesOutCount = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mOutgoingFlows.clear();
    mOutgoingFlows.reserve(*trustLinesOutCount);
    for (RecordNumber idx = 0; idx < *trustLinesOutCount; idx++) {
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
        mOutgoingFlows.push_back(make_pair(
            nodeUUID,
            make_shared<const TrustLineAmount>(trustLineAmount)));
    }
    //----------------------------------------------------
    RecordCount *trustLinesInCount = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mIncomingFlows.clear();
    mIncomingFlows.reserve(*trustLinesInCount);
    for (RecordNumber idx = 0; idx < *trustLinesInCount; idx++) {
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
        mIncomingFlows.push_back(make_pair(
            nodeUUID,
            make_shared<const TrustLineAmount>(trustLineAmount)));
    }
}

const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> ResultMaxFlowCalculationMessage::outgoingFlows() const {
    return mOutgoingFlows;
}

const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> ResultMaxFlowCalculationMessage::incomingFlows() const {
    return mIncomingFlows;
}

const bool ResultMaxFlowCalculationMessage::isMaxFlowCalculationResponseMessage() const {
    return true;
}

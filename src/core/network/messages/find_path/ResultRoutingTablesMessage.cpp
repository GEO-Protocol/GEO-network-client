#include "ResultRoutingTablesMessage.h"

ResultRoutingTablesMessage::ResultRoutingTablesMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    vector<NodeUUID> &rt1,
    unordered_map<NodeUUID, vector<NodeUUID>> &rt2,
    unordered_map<NodeUUID, vector<NodeUUID>> &rt3):

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mRT1(rt1),
    mRT2(rt2),
    mRT3(rt3){}

ResultRoutingTablesMessage::ResultRoutingTablesMessage(
        BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ResultRoutingTablesMessage::typeID() const {
    return Message::MessageTypeID::ResultRoutingTablesMessageType;
}

vector<NodeUUID> ResultRoutingTablesMessage::rt1() {

    return mRT1;
}

unordered_map<NodeUUID, vector<NodeUUID>> ResultRoutingTablesMessage::rt2() {

    return mRT2;
}

unordered_map<NodeUUID, vector<NodeUUID>> ResultRoutingTablesMessage::rt3() {

    return mRT3;
}

pair<BytesShared, size_t> ResultRoutingTablesMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
            sizeof(RecordCount) + mRT1.size() * NodeUUID::kBytesSize +
            rt2ByteSize() + rt3ByteSize();
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    RecordCount rt1Size = (RecordCount)mRT1.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &rt1Size,
        sizeof(RecordCount));
    dataBytesOffset += sizeof(RecordCount);
    //----------------------------------------------------
    for (auto const &itRT1 : mRT1) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            itRT1.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
    RecordCount rt2Size = (RecordCount)mRT2.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &rt2Size,
        sizeof(RecordCount));
    dataBytesOffset += sizeof(RecordCount);
    //----------------------------------------------------
    for (auto const &itRT2 : mRT2) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            itRT2.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        RecordCount rt2SizeVect = (RecordCount)itRT2.second.size();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &rt2SizeVect,
            sizeof(RecordCount));
        dataBytesOffset += sizeof(RecordCount);
        //------------------------------------------------
        for (auto const &nodeUUID : itRT2.second) {
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                nodeUUID.data,
                NodeUUID::kBytesSize);
            dataBytesOffset += NodeUUID::kBytesSize;
        }
    }
    //----------------------------------------------------
    RecordCount rt3Size = (RecordCount)mRT3.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &rt3Size,
        sizeof(RecordCount));
    dataBytesOffset += sizeof(RecordCount);
    //----------------------------------------------------
    for (auto const &itRT3 : mRT3) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            itRT3.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        RecordCount rt3SizeVect = (RecordCount)itRT3.second.size();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &rt3SizeVect,
            sizeof(RecordCount));
        dataBytesOffset += sizeof(RecordCount);
        //------------------------------------------------
        for (auto const &nodeUUID : itRT3.second) {
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                nodeUUID.data,
                NodeUUID::kBytesSize);
            dataBytesOffset += NodeUUID::kBytesSize;
        }
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void ResultRoutingTablesMessage::deserializeFromBytes(
        BytesShared buffer){

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    RecordCount *rt1Count = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mRT1.clear();
    mRT1.reserve(*rt1Count);
    for (RecordNumber idx = 0; idx < *rt1Count; idx++) {
        NodeUUID nodeUUID;
        memcpy(
            nodeUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        mRT1.push_back(nodeUUID);
    }
    //-----------------------------------------------------
    RecordCount *rt2Count = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mRT2.clear();
    mRT2.reserve(*rt2Count);
    for (RecordNumber idx = 0; idx < *rt2Count; idx++) {
        NodeUUID keyDesitnation;
        memcpy(
            keyDesitnation.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        RecordCount *rt2VectCount = new (buffer.get() + bytesBufferOffset) RecordCount;
        bytesBufferOffset += sizeof(RecordCount);
        //---------------------------------------------------
        vector<NodeUUID> valueSources;
        valueSources.reserve(*rt2VectCount);
        for (RecordNumber jdx = 0; jdx < *rt2VectCount; jdx++) {
            NodeUUID source;
            memcpy(
                source.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize);
            bytesBufferOffset += NodeUUID::kBytesSize;
            valueSources.push_back(source);
        }
        //---------------------------------------------------
        mRT2.insert(make_pair(keyDesitnation, valueSources));
    }
    //-----------------------------------------------------
    RecordCount *rt3Count = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mRT3.clear();
    mRT3.reserve(*rt3Count);
    for (RecordNumber idx = 0; idx < *rt3Count; idx++) {
        NodeUUID keyDesitnation;
        memcpy(
            keyDesitnation.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        RecordCount *rt3VectCount = new (buffer.get() + bytesBufferOffset) RecordCount;
        bytesBufferOffset += sizeof(RecordCount);
        //---------------------------------------------------
        vector<NodeUUID> valueSources;
        valueSources.reserve(*rt3VectCount);
        for (RecordNumber jdx = 0; jdx < *rt3VectCount; jdx++) {
            NodeUUID source;
            memcpy(
                source.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize);
            bytesBufferOffset += NodeUUID::kBytesSize;
            valueSources.push_back(source);
        }
        //---------------------------------------------------
        mRT3.insert(make_pair(keyDesitnation, valueSources));
    }
    //-----------------------------------------------------
}

size_t ResultRoutingTablesMessage::rt2ByteSize() {

    size_t result = sizeof(RecordCount);
    for (auto const &nodeUUIDAndVector : mRT2) {
        result += NodeUUID::kBytesSize + sizeof(RecordCount) +
                nodeUUIDAndVector.second.size() * NodeUUID::kBytesSize;
    }
    return result;
}

size_t ResultRoutingTablesMessage::rt3ByteSize() {

    size_t result = sizeof(RecordCount);
    for (auto const &nodeUUIDAndVector : mRT3) {
        result += NodeUUID::kBytesSize + sizeof(RecordCount) +
                  nodeUUIDAndVector.second.size() * NodeUUID::kBytesSize;
    }
    return result;
}
#include "ResultRoutingTable3LevelVectorMessage.h"

ResultRoutingTable3LevelVectorMessage::ResultRoutingTable3LevelVectorMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    vector<pair<NodeUUID, NodeUUID>> &rt3):

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mRT3(rt3){}

ResultRoutingTable3LevelVectorMessage::ResultRoutingTable3LevelVectorMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ResultRoutingTable3LevelVectorMessage::typeID() const {
    return Message::MessageTypeID::ResultRoutingTable3LevelVectorMessageType;
}

vector<pair<NodeUUID, NodeUUID>>& ResultRoutingTable3LevelVectorMessage::rt3() {

    return mRT3;
}

pair<BytesShared, size_t> ResultRoutingTable3LevelVectorMessage::serializeToBytes() {

    cout << "ResultRoutingTable3LevelVectorMessage::serializeToBytes start serializing" << endl;
    cout << "ResultRoutingTable3LevelVecotrMessage::serializeToBytes rt3 size: " << mRT3.size() << endl;
    DateTime startTime = utc_now();
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second + rt3ByteSize();
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
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
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            itRT3.second.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
    cout << "ResultRoutingTable3LevelVectorMessage::serializeToBytes message size: " << bytesCount << endl;
    Duration methodTime = utc_now() - startTime;
    cout << "ResultRoutingTable3LevelVectorMessage::serializing time: " << methodTime << endl;
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void ResultRoutingTable3LevelVectorMessage::deserializeFromBytes(
    BytesShared buffer){

    cout << "ResultRoutingTable3LevelVectorMessage::deserializeFromBytes start deserializing" << endl;
    DateTime startTime = utc_now();
    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //-----------------------------------------------------
    RecordCount *rt3Count = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mRT3.clear();
    mRT3.reserve(*rt3Count);
    for (RecordNumber idx = 0; idx < *rt3Count; idx++) {
        vector<byte> sourceBufferBytes(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + NodeUUID::kBytesSize);
        NodeUUID source(sourceBufferBytes.data());
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        vector<byte> destinationBufferBytes(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + NodeUUID::kBytesSize);
        NodeUUID destination(destinationBufferBytes.data());
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        mRT3.push_back(
            make_pair(
                source,
                destination));
    }
    //-----------------------------------------------------
    cout << "ResultRoutingTable3LevelVectorMessage::deserializeFromBytes message size: " << bytesBufferOffset << endl;
    Duration methodTime = utc_now() - startTime;
    cout << "ResultRoutingTable3LevelVectorMessage::deserializeFromBytes time: " << methodTime << endl;
}

size_t ResultRoutingTable3LevelVectorMessage::rt3ByteSize() {

    size_t result = sizeof(RecordCount) + mRT3.size() * (2 * NodeUUID::kBytesSize);
    return result;
}

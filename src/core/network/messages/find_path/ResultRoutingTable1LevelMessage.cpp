#include "ResultRoutingTable1LevelMessage.h"

ResultRoutingTable1LevelMessage::ResultRoutingTable1LevelMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    vector<NodeUUID> &rt1):

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mRT1(rt1) {}

ResultRoutingTable1LevelMessage::ResultRoutingTable1LevelMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ResultRoutingTable1LevelMessage::typeID() const {
    return Message::MessageTypeID::ResultRoutingTable1LevelMessageType;
}

vector<NodeUUID>& ResultRoutingTable1LevelMessage::rt1() {

    return mRT1;
}

pair<BytesShared, size_t> ResultRoutingTable1LevelMessage::serializeToBytes() {

    /*cout << "ResultRoutingTable1LevelMessage::serializeToBytes start serializing" << endl;
    cout << "ResultRoutingTable1LevelMessage::serializeToBytes rt1 size: " << mRT1.size() << endl;
    DateTime startTime = utc_now();*/
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
                        sizeof(RecordCount) + mRT1.size() * NodeUUID::kBytesSize;
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
    /*cout << "ResultRoutingTable1LevelMessage::serializeToBytes message size: " << bytesCount << endl;
    Duration methodTime = utc_now() - startTime;
    cout << "ResultRoutingTable1LevelMessage::serializing time: " << methodTime << endl;*/
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void ResultRoutingTable1LevelMessage::deserializeFromBytes(
    BytesShared buffer){

    /*cout << "ResultRoutingTable1LevelMessage::deserializeFromBytes start deserializing" << endl;
    DateTime startTime = utc_now();*/
    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    RecordCount *rt1Count = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mRT1.clear();
    mRT1.reserve(*rt1Count);
    for (RecordNumber idx = 0; idx < *rt1Count; idx++) {
        vector<byte> nodeUUIDBufferByte(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + NodeUUID::kBytesSize);
        NodeUUID nodeUUID(nodeUUIDBufferByte.data());
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        mRT1.push_back(nodeUUID);
    }
    //-----------------------------------------------------
    /*cout << "ResultRoutingTablesMessage::deserializeFromBytes message size: " << bytesBufferOffset << endl;
    Duration methodTime = utc_now() - startTime;
    cout << "ResultRoutingTablesMessage::deserializeFromBytes time: " << methodTime << endl;*/
}

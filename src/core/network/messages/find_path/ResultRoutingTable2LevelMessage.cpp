#include "ResultRoutingTable2LevelMessage.h"

ResultRoutingTable2LevelMessage::ResultRoutingTable2LevelMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &rt2):

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mRT2(rt2) {}

ResultRoutingTable2LevelMessage::ResultRoutingTable2LevelMessage(
    BytesShared buffer) :

    TransactionMessage(buffer)
{

    /*cout << "ResultRoutingTable2LevelMessage::deserializeFromBytes start serializing" << endl;
    cout << "ResultRoutingTable2LevelMessage::deserializeFromBytes rt2 size: " << mRT2.size() << endl;
    DateTime startTime = utc_now();*/
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    RecordCount *rt2Count = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mRT2.reserve(*rt2Count);
    for (RecordNumber idx = 0; idx < *rt2Count; idx++) {
        NodeUUID keyDesitnation(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        RecordCount *rt2VectCount = new (buffer.get() + bytesBufferOffset) RecordCount;
        bytesBufferOffset += sizeof(RecordCount);
        //---------------------------------------------------
        vector<NodeUUID> valueSources;
        valueSources.reserve(*rt2VectCount);
        for (RecordNumber jdx = 0; jdx < *rt2VectCount; jdx++) {
            NodeUUID source(buffer.get() + bytesBufferOffset);
            bytesBufferOffset += NodeUUID::kBytesSize;
            valueSources.push_back(source);
        }
        //---------------------------------------------------
        mRT2.insert(make_pair(keyDesitnation, valueSources));
    }
    cout << "ResultRoutingTable2LevelMessage::deserializeFromBytes message size: " << bytesBufferOffset << endl;
//    Duration methodTime = utc_now() - startTime;
//    cout << "ResultRoutingTable2LevelMessage::deserializing time: " << methodTime << endl;
}

const Message::MessageType ResultRoutingTable2LevelMessage::typeID() const {
    return Message::MessageType::Paths_ResultRoutingTableSecondLevel;
}

unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>>& ResultRoutingTable2LevelMessage::rt2() {

    return mRT2;
}

pair<BytesShared, size_t> ResultRoutingTable2LevelMessage::serializeToBytes() const
    throw(bad_alloc)
{

    /*cout << "ResultRoutingTable2LevelMessage::serializeToBytes start serializing" << endl;
    cout << "ResultRoutingTable2LevelMessage::serializeToBytes rt2 size: " << mRT2.size() << endl;
    DateTime startTime = utc_now();*/
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second + rt2ByteSize();
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
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
    cout << "ResultRoutingTable2LevelMessage::serializeToBytes message size: " << bytesCount << endl;
//    Duration methodTime = utc_now() - startTime;
//    cout << "ResultRoutingTable2LevelMessage::serializing time: " << methodTime << endl;
    return make_pair(
        dataBytesShared,
        bytesCount);
}


size_t ResultRoutingTable2LevelMessage::rt2ByteSize() const {

    size_t result = sizeof(RecordCount);
    for (auto const &nodeUUIDAndVector : mRT2) {
        result += NodeUUID::kBytesSize + sizeof(RecordCount) +
                  nodeUUIDAndVector.second.size() * NodeUUID::kBytesSize;
    }
    return result;
}

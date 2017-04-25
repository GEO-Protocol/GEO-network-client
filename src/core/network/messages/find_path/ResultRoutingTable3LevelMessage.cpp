#include "ResultRoutingTable3LevelMessage.h"

ResultRoutingTable3LevelMessage::ResultRoutingTable3LevelMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &rt3):

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mRT3(rt3){}

ResultRoutingTable3LevelMessage::ResultRoutingTable3LevelMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
#ifdef GETTING_PATHS_DEBUG_LOG
    /*cout << "ResultRoutingTable3LevelMessage::deserializeFromBytes start serializing" << endl;
    cout << "ResultRoutingTable3LevelMessage::deserializeFromBytes rt3 size: " << mRT3.size() << endl;*/
    DateTime startTime = utc_now();
#endif
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //-----------------------------------------------------
    RecordCount *rt3Count = new (buffer.get() + bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mRT3.reserve(*rt3Count);
    for (RecordNumber idx = 0; idx < *rt3Count; idx++) {
        NodeUUID keyDesitnation(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        RecordCount *rt3VectCount = new (buffer.get() + bytesBufferOffset) RecordCount;
        bytesBufferOffset += sizeof(RecordCount);
        //---------------------------------------------------
        vector<NodeUUID> valueSources;
        valueSources.reserve(*rt3VectCount);
        for (RecordNumber jdx = 0; jdx < *rt3VectCount; jdx++) {
            NodeUUID source(buffer.get() + bytesBufferOffset);
            bytesBufferOffset += NodeUUID::kBytesSize;
            valueSources.push_back(source);
        }
        //---------------------------------------------------
        mRT3.insert(make_pair(keyDesitnation, valueSources));
    }
#ifdef GETTING_PATHS_DEBUG_LOG
    cout << "ResultRoutingTable3LevelMessage::deserializeFromBytes message size: " << bytesBufferOffset << endl;
//    Duration methodTime = utc_now() - startTime;
//    cout << "ResultRoutingTable3LevelMessage::deserializing time: " << methodTime << endl;
#endif
}

const Message::MessageType ResultRoutingTable3LevelMessage::typeID() const {
    return Message::MessageType::Paths_ResultRoutingTableThirdLevel;
}

unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>>& ResultRoutingTable3LevelMessage::rt3() {

    return mRT3;
}

pair<BytesShared, size_t> ResultRoutingTable3LevelMessage::serializeToBytes() const
    throw(bad_alloc)
{
#ifdef GETTING_PATHS_DEBUG_LOG
    /*cout << "ResultRoutingTable3LevelMessage::serializeToBytes start serializing" << endl;
    cout << "ResultRoutingTable3LevelMessage::serializeToBytes rt3 size: " << mRT3.size() << endl;*/
    DateTime startTime = utc_now();
#endif
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
//#ifdef GETTING_PATHS_DEBUG_LOG
    cout << "ResultRoutingTable3LevelMessage::serializeToBytes message size: " << bytesCount << endl;
//    Duration methodTime = utc_now() - startTime;
//    cout << "ResultRoutingTable3LevelMessage::serializing time: " << methodTime << endl;
//#endif
    return make_pair(
        dataBytesShared,
        bytesCount);
}

size_t ResultRoutingTable3LevelMessage::rt3ByteSize() const {
    size_t result = sizeof(RecordCount);
    for (auto const &nodeUUIDAndVector : mRT3) {
        result += NodeUUID::kBytesSize + sizeof(RecordCount) +
                  nodeUUIDAndVector.second.size() * NodeUUID::kBytesSize;
    }
    return result;
}

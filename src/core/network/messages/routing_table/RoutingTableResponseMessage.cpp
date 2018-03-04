#include "RoutingTableResponseMessage.h"

RoutingTableResponseMessage::RoutingTableResponseMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &sender,
    set<NodeUUID> neighbors):
    SenderMessage(
        equivalent,
        sender),
    mNeighbors(neighbors)
{}

RoutingTableResponseMessage::RoutingTableResponseMessage(
    BytesShared buffer):
    SenderMessage(buffer)
{
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    SerializedRecordsCount *neighborsNumber = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    for (SerializedRecordNumber idx = 0; idx < *neighborsNumber; idx++) {
        NodeUUID nodeUUID(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighbors.insert(nodeUUID);
    }
}

const Message::MessageType RoutingTableResponseMessage::typeID() const
{
    return Message::MessageType::RoutingTableResponse;
}

pair<BytesShared, size_t> RoutingTableResponseMessage::serializeToBytes() const
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();
    SerializedRecordsCount neighborsSize = (SerializedRecordsCount)mNeighbors.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount)
                        + neighborsSize * (NodeUUID::kBytesSize);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &neighborsSize,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);
    for(const auto &kNodeUUUID: mNeighbors){
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &kNodeUUUID,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

set<NodeUUID> RoutingTableResponseMessage::neighbors() const
{
    return mNeighbors;
}

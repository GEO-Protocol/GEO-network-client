#include "RoutingTableResponseMessage.h"

const Message::MessageType RoutingTableResponseMessage::typeID() const
{
    return Message::MessageType::RoutingTableResponse;
}

RoutingTableResponseMessage::RoutingTableResponseMessage(
    const NodeUUID &sender,
    set<NodeUUID> neighbors):
    SenderMessage(sender),
    mNeighbors(neighbors)
{

}

RoutingTableResponseMessage::RoutingTableResponseMessage(BytesShared buffer):
    SenderMessage(buffer)
{
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    uint16_t *neighborsNumber = new (buffer.get() + bytesBufferOffset) uint16_t;
    bytesBufferOffset += sizeof(uint16_t);
    for (auto idx = 0; idx < *neighborsNumber; idx++) {
        NodeUUID nodeUUID(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighbors.insert(nodeUUID);
    }
}

pair<BytesShared, size_t> RoutingTableResponseMessage::serializeToBytes() const
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();
    uint16_t neighborsSize = mNeighbors.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(uint16_t)
                        + neighborsSize * (NodeUUID::kBytesSize);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &neighborsSize,
        sizeof(uint16_t)
    );
    dataBytesOffset += sizeof(uint16_t);
    for(const auto &kNodeUUUID: mNeighbors){
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &kNodeUUUID,
            NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

set<NodeUUID> RoutingTableResponseMessage::neighbors() const {
    return mNeighbors;
}

#include "CyclesThreeNodesBalancesRequestMessage.h"


CyclesThreeNodesBalancesRequestMessage::CyclesThreeNodesBalancesRequestMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    set<NodeUUID> &neighbors):

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mNeighbors(neighbors)
{}

CyclesThreeNodesBalancesRequestMessage::CyclesThreeNodesBalancesRequestMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    SerializedRecordsCount neighborsCount;
    // path
    memcpy(
        &neighborsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber i = 1; i <= neighborsCount; ++i) {
        NodeUUID stepNode(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighbors.insert(stepNode);
    }
}

pair<BytesShared, size_t> CyclesThreeNodesBalancesRequestMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    SerializedRecordsCount neighborsCount = (SerializedRecordsCount)mNeighbors.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount)
                        + neighborsCount * NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    // For path
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &neighborsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for(auto const& value: mNeighbors) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &value,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType CyclesThreeNodesBalancesRequestMessage::typeID() const
{
    return Message::MessageType::Cycles_ThreeNodesBalancesRequest;
}

set<NodeUUID> CyclesThreeNodesBalancesRequestMessage::neighbors()
{
    return mNeighbors;
}

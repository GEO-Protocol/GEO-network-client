#include "RoutingTableResponseMessage.h"

RoutingTableResponseMessage::RoutingTableResponseMessage(
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    vector<pair<SerializedEquivalent, set<NodeUUID>>> neighbors):
    ConfirmationMessage(
        0,
        sender,
        transactionUUID),
    mNeighbors(neighbors)
{}

RoutingTableResponseMessage::RoutingTableResponseMessage(
    BytesShared buffer):
    ConfirmationMessage(buffer)
{
    size_t bytesBufferOffset = ConfirmationMessage::kOffsetToInheritedBytes();
    auto *equivalentsNumber = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    for (SerializedRecordNumber idxEq = 0; idxEq < *equivalentsNumber; idxEq++) {
        SerializedEquivalent equivalentTmp;
        memcpy(
            &equivalentTmp,
            buffer.get() + bytesBufferOffset,
            sizeof(SerializedEquivalent));
        bytesBufferOffset += sizeof(SerializedEquivalent);
        auto *neighborsNumber = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
        bytesBufferOffset += sizeof(SerializedRecordsCount);
        set<NodeUUID> neighbors;
        for (SerializedRecordNumber idx = 0; idx < *neighborsNumber; idx++) {
            NodeUUID nodeUUID(buffer.get() + bytesBufferOffset);
            bytesBufferOffset += NodeUUID::kBytesSize;
            neighbors.insert(nodeUUID);
        }
        mNeighbors.emplace_back(
            equivalentTmp,
            neighbors);
    }
}

const Message::MessageType RoutingTableResponseMessage::typeID() const
{
    return Message::MessageType::RoutingTableResponse;
}

pair<BytesShared, size_t> RoutingTableResponseMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = ConfirmationMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount);
    for (const auto &equivalentAndNeighbors : mNeighbors) {
        auto neighborsSize = (SerializedRecordsCount)equivalentAndNeighbors.second.size();
        bytesCount += sizeof(SerializedEquivalent) + sizeof(SerializedRecordsCount)
                      + neighborsSize * (NodeUUID::kBytesSize);
    }

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    auto equivalentsCount = (SerializedRecordsCount)mNeighbors.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &equivalentsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for (const auto &equivalentAndNeighbors : mNeighbors) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &equivalentAndNeighbors.first,
            sizeof(SerializedEquivalent));
        dataBytesOffset += sizeof(SerializedEquivalent);

        auto neighborsCount = (SerializedRecordsCount)equivalentAndNeighbors.second.size();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &neighborsCount,
            sizeof(SerializedRecordsCount));
        dataBytesOffset += sizeof(SerializedRecordsCount);
        for (const auto &kNodeUUUID: equivalentAndNeighbors.second) {
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &kNodeUUUID,
                NodeUUID::kBytesSize);
            dataBytesOffset += NodeUUID::kBytesSize;
        }
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

vector<pair<SerializedEquivalent, set<NodeUUID>>> RoutingTableResponseMessage::neighborsByEquivalents() const
{
    return mNeighbors;
}

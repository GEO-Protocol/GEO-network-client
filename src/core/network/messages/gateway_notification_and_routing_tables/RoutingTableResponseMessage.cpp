#include "RoutingTableResponseMessage.h"

RoutingTableResponseMessage::RoutingTableResponseMessage(
    ContractorID idOnReceiverSide,
    const TransactionUUID &transactionUUID,
    vector<pair<SerializedEquivalent, vector<BaseAddress::Shared>>> neighbors):
    ConfirmationMessage(
        0,
        idOnReceiverSide,
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
    mNeighbors.reserve(*equivalentsNumber);
    for (SerializedRecordNumber idxEq = 0; idxEq < *equivalentsNumber; idxEq++) {
        SerializedEquivalent equivalentTmp;
        memcpy(
            &equivalentTmp,
            buffer.get() + bytesBufferOffset,
            sizeof(SerializedEquivalent));
        bytesBufferOffset += sizeof(SerializedEquivalent);
        auto *neighborsNumber = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
        bytesBufferOffset += sizeof(SerializedRecordsCount);
        vector<BaseAddress::Shared> neighborsAddresses;
        neighborsAddresses.reserve(*neighborsNumber);
        for (SerializedRecordNumber idx = 0; idx < *neighborsNumber; idx++) {
            auto address = deserializeAddress(
                buffer.get() + bytesBufferOffset);
            bytesBufferOffset += address->serializedSize();
            neighborsAddresses.push_back(address);
        }
        mNeighbors.emplace_back(
            equivalentTmp,
            neighborsAddresses);
    }
}

const Message::MessageType RoutingTableResponseMessage::typeID() const
{
    return Message::MessageType::RoutingTableResponse;
}

pair<BytesShared, size_t> RoutingTableResponseMessage::serializeToBytes() const
{
    auto parentBytesAndCount = ConfirmationMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount);
    for (const auto &equivalentAndNeighbors : mNeighbors) {
        bytesCount += sizeof(SerializedEquivalent) + sizeof(SerializedRecordsCount);
        for (const auto &neighborAddress : equivalentAndNeighbors.second) {
            bytesCount += neighborAddress->serializedSize();
        }
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
        for (const auto &neighborAddress: equivalentAndNeighbors.second) {
            auto serializeAddress = neighborAddress->serializeToBytes();
            memcpy(
                dataBytesShared.get() + dataBytesOffset,
                serializeAddress.get(),
                neighborAddress->serializedSize());
            dataBytesOffset += neighborAddress->serializedSize();
        }
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

vector<pair<SerializedEquivalent, vector<BaseAddress::Shared>>> RoutingTableResponseMessage::neighborsByEquivalents() const
{
    return mNeighbors;
}

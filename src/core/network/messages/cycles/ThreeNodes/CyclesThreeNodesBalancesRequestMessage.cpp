#include "CyclesThreeNodesBalancesRequestMessage.h"


CyclesThreeNodesBalancesRequestMessage::CyclesThreeNodesBalancesRequestMessage(
    const SerializedEquivalent equivalent,
    ContractorID idOnReceiverSide,
    const TransactionUUID &transactionUUID,
    vector<BaseAddress::Shared> &neighbors):

    TransactionMessage(
        equivalent,
        idOnReceiverSide,
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

    for (SerializedRecordNumber idx = 0; idx < neighborsCount; idx++) {
        auto stepAddress = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += stepAddress->serializedSize();
        mNeighbors.push_back(stepAddress);
    }
}

pair<BytesShared, size_t> CyclesThreeNodesBalancesRequestMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    auto neighborsCount = (SerializedRecordsCount)mNeighbors.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount);
    for (const auto &address : mNeighbors) {
        bytesCount += address->serializedSize();
    }

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

    for(auto const &address: mNeighbors) {
        auto serializedData = address->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedData.get(),
            address->serializedSize());
        dataBytesOffset += address->serializedSize();
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

vector<BaseAddress::Shared> CyclesThreeNodesBalancesRequestMessage::neighbors()
{
    return mNeighbors;
}

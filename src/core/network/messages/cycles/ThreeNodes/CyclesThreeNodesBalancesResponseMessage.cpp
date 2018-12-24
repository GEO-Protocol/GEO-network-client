#include "CyclesThreeNodesBalancesResponseMessage.h"


CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(
    const SerializedEquivalent equivalent,
    ContractorID idOnReceiverSide,
    const TransactionUUID &transactionUUID,
    vector<BaseAddress::Shared> &neighbors) :
    TransactionMessage(
        equivalent,
        idOnReceiverSide,
        transactionUUID),
    mNeighbors(neighbors)
{}

CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //    Get NodesCount
    SerializedRecordsCount boundaryNodesCount;
    memcpy(
        &boundaryNodesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //    Parse boundary nodes
    for (SerializedRecordNumber idx = 0; idx < boundaryNodesCount; idx++){
        auto stepAddress = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += stepAddress->serializedSize();
        mNeighbors.push_back(stepAddress);
    }
}

std::pair<BytesShared, size_t> CyclesThreeNodesBalancesResponseMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    auto boundaryNodesCount = (SerializedRecordsCount) mNeighbors.size();
    size_t bytesCount =
        parentBytesAndCount.second +
        sizeof(SerializedRecordsCount);
    for (const auto &address : mNeighbors) {
        bytesCount += address->serializedSize();
    }
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    // for parent node
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //    for mNeighborsBalances
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &boundaryNodesCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);
    for(const auto &kNodeAddress: mNeighbors) {
        auto serializedData = kNodeAddress->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedData.get(),
            kNodeAddress->serializedSize());
        dataBytesOffset += kNodeAddress->serializedSize();
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType CyclesThreeNodesBalancesResponseMessage::typeID() const
    noexcept
{
    return Message::MessageType::Cycles_ThreeNodesBalancesResponse;
}

vector<BaseAddress::Shared> CyclesThreeNodesBalancesResponseMessage::commonNodes()
    noexcept
{
    return mNeighbors;
}

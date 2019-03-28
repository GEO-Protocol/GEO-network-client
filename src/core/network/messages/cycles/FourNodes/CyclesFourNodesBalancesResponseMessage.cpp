#include "CyclesFourNodesBalancesResponseMessage.h"

CyclesFourNodesBalancesResponseMessage::CyclesFourNodesBalancesResponseMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID &transactionUUID,
    vector<BaseAddress::Shared> &suitableNodes):

    TransactionMessage(
        equivalent,
        senderAddresses,
        transactionUUID),
    mSuitableNodes(suitableNodes)
{}

CyclesFourNodesBalancesResponseMessage::CyclesFourNodesBalancesResponseMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    SerializedRecordsCount suitableNodesCount;
    memcpy(
        &suitableNodesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber idx = 0; idx < suitableNodesCount; idx++) {
        auto stepAddress = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += stepAddress->serializedSize();
        mSuitableNodes.push_back(stepAddress);
    }
}

pair<BytesShared, size_t> CyclesFourNodesBalancesResponseMessage::serializeToBytes() const
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    auto contractorsCount = (SerializedRecordsCount)mSuitableNodes.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount);
    for (const auto &address : mSuitableNodes) {
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

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &contractorsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for(auto const &address: mSuitableNodes) {
        auto serializedAddress = address->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedAddress.get(),
            address->serializedSize());
        dataBytesOffset += address->serializedSize();
    }

    return make_pair(
        dataBytesShared,
        bytesCount);
}

vector<BaseAddress::Shared> CyclesFourNodesBalancesResponseMessage::suitableNodes() const
{
    return mSuitableNodes;
}

const Message::MessageType CyclesFourNodesBalancesResponseMessage::typeID() const
{
    return Message::MessageType::Cycles_FourNodesBalancesResponse;
}
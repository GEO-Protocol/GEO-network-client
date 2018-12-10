#include "MaxFlowCalculationMessage.h"


MaxFlowCalculationMessage::MaxFlowCalculationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    ContractorID idOnReceiverSide,
    const NodeUUID& targetUUID,
    vector<BaseAddress::Shared> targetAddresses) :

    SenderMessage(
        equivalent,
        senderUUID,
        idOnReceiverSide),

    mTargetUUID(targetUUID),
    mTargetAddresses(targetAddresses)
{}

MaxFlowCalculationMessage::MaxFlowCalculationMessage (
    BytesShared buffer) :
    SenderMessage(buffer)
{
    auto bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();

    memcpy(
        mTargetUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    uint16_t senderAddressesCnt;
    memcpy(
        &senderAddressesCnt,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
    bytesBufferOffset += sizeof(byte);

    for (int idx = 0; idx < senderAddressesCnt; idx++) {
        auto targetAddress = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        mTargetAddresses.push_back(targetAddress);
        bytesBufferOffset += targetAddress->serializedSize();
    }
}

const NodeUUID &MaxFlowCalculationMessage::targetUUID() const
{
    return mTargetUUID;
}

vector<BaseAddress::Shared> MaxFlowCalculationMessage::targetAddresses() const
{
    return mTargetAddresses;
}

pair<BytesShared, size_t> MaxFlowCalculationMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + sizeof(byte);
    for (const auto &address : mTargetAddresses) {
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
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mTargetUUID.data,
        NodeUUID::kBytesSize);
    dataBytesOffset += NodeUUID::kBytesSize;
    //----------------------------------------------------
    auto targetAddressesCnt = (byte)mTargetAddresses.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &targetAddressesCnt,
        sizeof(byte));
    dataBytesOffset += sizeof(byte);

    for (auto &targetAddress : mTargetAddresses) {
        auto serializedData = targetAddress->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedData.get(),
            targetAddress->serializedSize());
        dataBytesOffset += targetAddress->serializedSize();
    }

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t MaxFlowCalculationMessage::kOffsetToInheritedBytes() const
    noexcept
{
    auto kOffset = SenderMessage::kOffsetToInheritedBytes()
           + NodeUUID::kBytesSize
           + sizeof(byte);
    for (const auto &address : mTargetAddresses) {
        kOffset += address->serializedSize();
    }
    return kOffset;
}

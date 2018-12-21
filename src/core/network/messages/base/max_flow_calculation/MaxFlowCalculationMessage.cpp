#include "MaxFlowCalculationMessage.h"


MaxFlowCalculationMessage::MaxFlowCalculationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    ContractorID idOnReceiverSide,
    vector<BaseAddress::Shared> targetAddresses) :

    SenderMessage(
        equivalent,
        senderUUID,
        idOnReceiverSide),

    mTargetAddresses(targetAddresses)
{}

MaxFlowCalculationMessage::MaxFlowCalculationMessage (
    BytesShared buffer) :
    SenderMessage(buffer)
{
    auto bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();

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

vector<BaseAddress::Shared> MaxFlowCalculationMessage::targetAddresses() const
{
    return mTargetAddresses;
}

pair<BytesShared, size_t> MaxFlowCalculationMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
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
           + sizeof(byte);
    for (const auto &address : mTargetAddresses) {
        kOffset += address->serializedSize();
    }
    return kOffset;
}

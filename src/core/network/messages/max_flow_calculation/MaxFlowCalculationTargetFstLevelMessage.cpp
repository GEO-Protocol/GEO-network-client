#include "MaxFlowCalculationTargetFstLevelMessage.h"

MaxFlowCalculationTargetFstLevelMessage::MaxFlowCalculationTargetFstLevelMessage(
    const SerializedEquivalent equivalent,
    ContractorID idOnReceiverSide,
    vector<BaseAddress::Shared> targetAddresses,
    bool isTargetGateway)
    noexcept:
    MaxFlowCalculationMessage(
        equivalent,
        idOnReceiverSide,
        targetAddresses),
    mIsTargetGateway(isTargetGateway)
{}

MaxFlowCalculationTargetFstLevelMessage::MaxFlowCalculationTargetFstLevelMessage(
    BytesShared buffer)
    noexcept:
    MaxFlowCalculationMessage(buffer)
{
    size_t bytesBufferOffset = MaxFlowCalculationMessage::kOffsetToInheritedBytes();

    memcpy(
        &mIsTargetGateway,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
}

bool MaxFlowCalculationTargetFstLevelMessage::isTargetGateway() const
{
    return mIsTargetGateway;
}

const Message::MessageType MaxFlowCalculationTargetFstLevelMessage::typeID() const
{
    return Message::MessageType::MaxFlow_CalculationTargetFirstLevel;
}

pair<BytesShared, size_t> MaxFlowCalculationTargetFstLevelMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = MaxFlowCalculationMessage::serializeToBytes();
    size_t bytesCount =
        parentBytesAndCount.second +
        sizeof(byte);

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
        &mIsTargetGateway,
        sizeof(byte));

    return make_pair(
        dataBytesShared,
        bytesCount);
}

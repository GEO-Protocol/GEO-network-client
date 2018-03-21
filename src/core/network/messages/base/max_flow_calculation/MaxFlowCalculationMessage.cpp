#include "MaxFlowCalculationMessage.h"


MaxFlowCalculationMessage::MaxFlowCalculationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    SenderMessage(
        equivalent,
        senderUUID),

    mTargetUUID(targetUUID)
{}

MaxFlowCalculationMessage::MaxFlowCalculationMessage (
    BytesShared buffer) :
    SenderMessage(buffer)
{
    memcpy(
        mTargetUUID.data,
        buffer.get() + SenderMessage::kOffsetToInheritedBytes(),
        NodeUUID::kBytesSize);
}

const NodeUUID &MaxFlowCalculationMessage::targetUUID() const
{
    return mTargetUUID;
}

pair<BytesShared, size_t> MaxFlowCalculationMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second + NodeUUID::kBytesSize;
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
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t MaxFlowCalculationMessage::kOffsetToInheritedBytes() const
    noexcept
{
    return  SenderMessage::kOffsetToInheritedBytes() + NodeUUID::kBytesSize;
}

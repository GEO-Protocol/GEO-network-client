#include "InitiateMaxFlowCalculationMessage.h"

InitiateMaxFlowCalculationMessage::InitiateMaxFlowCalculationMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> &senderAddresses,
    bool isSenderGateway)
    noexcept:
    SenderMessage(
        equivalent,
        senderAddresses),
    mIsSenderGateway(isSenderGateway)
{}

InitiateMaxFlowCalculationMessage::InitiateMaxFlowCalculationMessage(
    BytesShared buffer)
    noexcept:
    SenderMessage(buffer)
{
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();

    memcpy(
        &mIsSenderGateway,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
}

bool InitiateMaxFlowCalculationMessage::isSenderGateway() const
{
    return mIsSenderGateway;
}

const Message::MessageType InitiateMaxFlowCalculationMessage::typeID() const
{
    return Message::MaxFlow_InitiateCalculation;
}

pair<BytesShared, size_t> InitiateMaxFlowCalculationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();
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
        &mIsSenderGateway,
        sizeof(byte));

    return make_pair(
        dataBytesShared,
        bytesCount);
}

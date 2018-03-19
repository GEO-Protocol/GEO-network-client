#include "MaxFlowCalculationConfirmationMessage.h"

MaxFlowCalculationConfirmationMessage::MaxFlowCalculationConfirmationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const ConfirmationID confirmationID) :

    SenderMessage(
        equivalent,
        senderUUID),
    mConfirmationID(confirmationID)
{}

MaxFlowCalculationConfirmationMessage::MaxFlowCalculationConfirmationMessage(
    BytesShared buffer):

    SenderMessage(buffer)
{
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    ConfirmationID *confirmationID = new (buffer.get() + bytesBufferOffset) ConfirmationID;
    mConfirmationID = (ConfirmationID) (*confirmationID);
}

void MaxFlowCalculationConfirmationMessage::setConfirmationID(
    const ConfirmationID confirmationID)
{
    mConfirmationID = confirmationID;
}

const Message::MessageType MaxFlowCalculationConfirmationMessage::typeID() const
{
    return Message::MaxFlow_Confirmation;
}

const ConfirmationID MaxFlowCalculationConfirmationMessage::confirmationID() const
{
    return mConfirmationID;
}

pair<BytesShared, size_t> MaxFlowCalculationConfirmationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();

    size_t bytesCount =
        parentBytesAndCount.second
        + sizeof(ConfirmationID);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
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
        &mConfirmationID,
        sizeof(ConfirmationID));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t MaxFlowCalculationConfirmationMessage::kOffsetToInheritedBytes() const
    noexcept
{
    const auto kOffset =
        SenderMessage::kOffsetToInheritedBytes()
        + sizeof(ConfirmationID);
    return kOffset;
}

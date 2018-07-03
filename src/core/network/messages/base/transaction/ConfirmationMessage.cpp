#include "ConfirmationMessage.h"

ConfirmationMessage::ConfirmationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const OperationState state) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mState(state)
{}

ConfirmationMessage::ConfirmationMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    auto *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
}

const Message::MessageType ConfirmationMessage::typeID() const
{
    return Message::System_Confirmation;
}

const ConfirmationMessage::OperationState ConfirmationMessage::state() const
{
    return mState;
}

pair<BytesShared, size_t> ConfirmationMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount =
        parentBytesAndCount.second
        + sizeof(SerializedOperationState);

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
        &mState,
        sizeof(SerializedOperationState));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t ConfirmationMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const auto kOffset =
        TransactionMessage::kOffsetToInheritedBytes()
        + sizeof(SerializedOperationState);

    return kOffset;
}

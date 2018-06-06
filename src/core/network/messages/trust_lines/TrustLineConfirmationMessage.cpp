#include "TrustLineConfirmationMessage.h"

TrustLineConfirmationMessage::TrustLineConfirmationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    bool gateway,
    const OperationState state) :

    ConfirmationMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        state),
    mGateway(gateway)
{}

TrustLineConfirmationMessage::TrustLineConfirmationMessage(
    BytesShared buffer):

    ConfirmationMessage(buffer)
{
    size_t bytesBufferOffset = ConfirmationMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        &mGateway,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
}

const Message::MessageType TrustLineConfirmationMessage::typeID() const
{
    return Message::TrustLines_Confirmation;
}

const bool TrustLineConfirmationMessage::gateway() const
{
    return mGateway;
}

pair<BytesShared, size_t> TrustLineConfirmationMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = ConfirmationMessage::serializeToBytes();

    size_t bytesCount =
            parentBytesAndCount.second
            + sizeof(byte);

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
        &mGateway,
        sizeof(byte));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}
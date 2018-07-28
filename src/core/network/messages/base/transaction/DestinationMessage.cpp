#include "DestinationMessage.h"

DestinationMessage::DestinationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const NodeUUID &destinationUUID)
    noexcept :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mDestinationUUID(destinationUUID)
{}

DestinationMessage::DestinationMessage(
    BytesShared buffer)
    noexcept :

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    memcpy(
        mDestinationUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
}

const NodeUUID& DestinationMessage::destinationUUID() const
noexcept
{
    return mDestinationUUID;
}

const bool DestinationMessage::isDestinationMessage() const
{
    return true;
}

/*
 * ToDo: rewrite me with bytes deserializer
 */
pair<BytesShared, size_t> DestinationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                    + NodeUUID::kBytesSize;

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
        mDestinationUUID.data,
        NodeUUID::kBytesSize);
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t DestinationMessage::kOffsetToInheritedBytes() const
noexcept
{
    static const auto kOffset =
        TransactionMessage::kOffsetToInheritedBytes()
        + NodeUUID::kBytesSize;

    return kOffset;
}

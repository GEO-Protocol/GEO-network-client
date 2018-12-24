#include "DestinationMessage.h"

DestinationMessage::DestinationMessage(
    const SerializedEquivalent equivalent,
    ContractorID idOnSenderSide,
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID &transactionUUID,
    ContractorID destinationID)
    noexcept:

    TransactionMessage(
        equivalent,
        idOnSenderSide,
        senderAddresses,
        transactionUUID),
    mDestinationID(destinationID)
{}

DestinationMessage::DestinationMessage(
    const SerializedEquivalent equivalent,
    ContractorID idOnSenderSide,
    const TransactionUUID &transactionUUID,
    ContractorID destinationID)
    noexcept:

    TransactionMessage(
        equivalent,
        idOnSenderSide,
        transactionUUID),
    mDestinationID(destinationID)
{}

DestinationMessage::DestinationMessage(
    BytesShared buffer)
    noexcept :

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    memcpy(
        &mDestinationID,
        buffer.get() + bytesBufferOffset,
        sizeof(ContractorID));
}

const ContractorID DestinationMessage::destinationID() const
    noexcept
{
    return mDestinationID;
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
                    + sizeof(ContractorID);

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
        &mDestinationID,
        sizeof(ContractorID));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t DestinationMessage::kOffsetToInheritedBytes() const
noexcept
{
    const auto kOffset =
        TransactionMessage::kOffsetToInheritedBytes()
        + sizeof(ContractorID);

    return kOffset;
}

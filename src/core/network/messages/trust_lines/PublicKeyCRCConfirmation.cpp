#include "PublicKeyCRCConfirmation.h"

PublicKeyCRCConfirmation::PublicKeyCRCConfirmation(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    uint32_t number,
    uint64_t crcConfirmation):
    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mNumber(number),
    mCrcConfirmation(crcConfirmation)
{}

PublicKeyCRCConfirmation::PublicKeyCRCConfirmation(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    memcpy(
        &mNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(uint32_t));
    bytesBufferOffset += sizeof(uint32_t);

    memcpy(
        &mCrcConfirmation,
        buffer.get() + bytesBufferOffset,
        sizeof(uint64_t));
}

const Message::MessageType PublicKeyCRCConfirmation::typeID() const
    noexcept
{
    return Message::TrustLines_CRCConfirmation;
}

const uint32_t PublicKeyCRCConfirmation::number() const
{
    return mNumber;
}

const uint64_t PublicKeyCRCConfirmation::crcConfirmation() const
{
    return mCrcConfirmation;
}

pair<BytesShared, size_t> PublicKeyCRCConfirmation::serializeToBytes() const
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(uint32_t)
            + sizeof(uint64_t);
    BytesShared buffer = tryMalloc(kBufferSize);

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    memcpy(
        buffer.get() + dataBytesOffset,
        &mNumber,
        sizeof(uint32_t));
    dataBytesOffset += sizeof(uint32_t);

    memcpy(
        buffer.get() + dataBytesOffset,
        &mCrcConfirmation,
        sizeof(uint64_t));

    return make_pair(
        buffer,
        kBufferSize);
}
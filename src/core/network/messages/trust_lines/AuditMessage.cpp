#include "AuditMessage.h"

AuditMessage::AuditMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const uint32_t keyNumber,
    const size_t signedDataSize,
    BytesShared signedData):
    TransactionMessage(
         equivalent,
         senderUUID,
         transactionUUID),
    mSignedData(signedData),
    mSignedDataSize(signedDataSize),
    mKeyNumber(keyNumber)
{}

AuditMessage::AuditMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    memcpy(
        &mKeyNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(uint32_t));
    bytesBufferOffset += sizeof(uint32_t);
    memcpy(
        &mSignedDataSize,
        buffer.get() + bytesBufferOffset,
        sizeof(size_t));
    bytesBufferOffset += sizeof(size_t);
    mSignedData = tryMalloc(mSignedDataSize);
    memcpy(
        mSignedData.get(),
        buffer.get() + bytesBufferOffset,
        mSignedDataSize);
}

const Message::MessageType AuditMessage::typeID() const
{
    return Message::TrustLines_Audit;
}

const uint32_t AuditMessage::keyNumber() const
{
    return mKeyNumber;
}

const size_t AuditMessage::signedDataSize() const
{
    return mSignedDataSize;
}

BytesShared AuditMessage::signedData() const
{
    return mSignedData;
}

pair<BytesShared, size_t> AuditMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(uint32_t)
            + sizeof(size_t)
            + mSignedDataSize;
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
        &mKeyNumber,
        sizeof(uint32_t));
    dataBytesOffset += sizeof(uint32_t);

    memcpy(
        buffer.get() + dataBytesOffset,
        &mSignedDataSize,
        sizeof(size_t));
    dataBytesOffset += sizeof(size_t);

    memcpy(
        buffer.get() + dataBytesOffset,
        mSignedData.get(),
        mSignedDataSize);

    return make_pair(
        buffer,
        kBufferSize);
}
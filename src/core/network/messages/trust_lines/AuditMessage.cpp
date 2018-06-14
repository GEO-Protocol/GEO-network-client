#include "AuditMessage.h"

AuditMessage::AuditMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const KeyNumber keyNumber,
    const lamport::Signature::Shared signedData):
    TransactionMessage(
         equivalent,
         senderUUID,
         transactionUUID),
    mSignedData(signedData),
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
        sizeof(KeyNumber));
    bytesBufferOffset += sizeof(KeyNumber);

    mSignedData = make_shared<lamport::Signature>(
        buffer.get() + bytesBufferOffset);
}

const Message::MessageType AuditMessage::typeID() const
{
    return Message::TrustLines_Audit;
}

const uint32_t AuditMessage::keyNumber() const
{
    return mKeyNumber;
}

const lamport::Signature::Shared AuditMessage::signedData() const
{
    return mSignedData;
}

pair<BytesShared, size_t> AuditMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(KeyNumber)
            + mSignedData->signatureSize();
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
        sizeof(KeyNumber));
    dataBytesOffset += sizeof(KeyNumber);

    memcpy(
        buffer.get() + dataBytesOffset,
        mSignedData.get(),
        mSignedData->signatureSize());

    return make_pair(
        buffer,
        kBufferSize);
}
#include "AuditMessage.h"

AuditMessage::AuditMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const NodeUUID &destinationUUID,
    const KeyNumber keyNumber,
    const lamport::Signature::Shared signature):
    DestinationMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        destinationUUID),
    mSignature(signature),
    mKeyNumber(keyNumber)
{}

AuditMessage::AuditMessage(
    BytesShared buffer) :
    DestinationMessage(buffer)
{
    auto bytesBufferOffset = DestinationMessage::kOffsetToInheritedBytes();

    memcpy(
        &mKeyNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(KeyNumber));
    bytesBufferOffset += sizeof(KeyNumber);

    mSignature = make_shared<lamport::Signature>(
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

const lamport::Signature::Shared AuditMessage::signature() const
{
    return mSignature;
}

const bool AuditMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}

const bool AuditMessage::isCheckCachedResponse() const
{
    return true;
}

pair<BytesShared, size_t> AuditMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = DestinationMessage::serializeToBytes();
    auto kBufferSize = parentBytesAndCount.second
                       + sizeof(KeyNumber)
                       + mSignature->signatureSize();
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
        mSignature->data(),
        mSignature->signatureSize());

    return make_pair(
        buffer,
        kBufferSize);
}

const size_t AuditMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const auto kOffset =
            DestinationMessage::kOffsetToInheritedBytes()
            + sizeof(KeyNumber)
            + mSignature->signatureSize();
    return kOffset;
}
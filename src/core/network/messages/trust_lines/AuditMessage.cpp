#include "AuditMessage.h"

AuditMessage::AuditMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const KeyNumber keyNumber,
    const lamport::Signature::Shared signature):
    ConfirmationMessage(
         equivalent,
         senderUUID,
         transactionUUID),
    mSignature(signature),
    mKeyNumber(keyNumber)
{}

AuditMessage::AuditMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    OperationState state) :
    ConfirmationMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        state),
    mSignature(nullptr)
{}

AuditMessage::AuditMessage(
    BytesShared buffer) :
    ConfirmationMessage(buffer)
{
    auto bytesBufferOffset = ConfirmationMessage::kOffsetToInheritedBytes();

    if (state() == ConfirmationMessage::OK) {
        memcpy(
            &mKeyNumber,
            buffer.get() + bytesBufferOffset,
            sizeof(KeyNumber));
        bytesBufferOffset += sizeof(KeyNumber);

        mSignature = make_shared<lamport::Signature>(
            buffer.get() + bytesBufferOffset);
    }
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

pair<BytesShared, size_t> AuditMessage::serializeToBytes() const
    throw (bad_alloc)
{
    const auto parentBytesAndCount = ConfirmationMessage::serializeToBytes();
    auto kBufferSize = parentBytesAndCount.second;
    if (state() == ConfirmationMessage::OK) {
        kBufferSize += sizeof(KeyNumber) + mSignature->signatureSize();
    }
    BytesShared buffer = tryMalloc(kBufferSize);

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    if (state() == ConfirmationMessage::OK) {
        memcpy(
            buffer.get() + dataBytesOffset,
            &mKeyNumber,
            sizeof(KeyNumber));
        dataBytesOffset += sizeof(KeyNumber);

        memcpy(
            buffer.get() + dataBytesOffset,
            mSignature->data(),
            mSignature->signatureSize());
    }

    return make_pair(
        buffer,
        kBufferSize);
}
#include "AuditResponseMessage.h"

AuditResponseMessage::AuditResponseMessage(
    const SerializedEquivalent equivalent,
    Contractor::Shared contractor,
    const TransactionUUID &transactionUUID,
    const KeyNumber keyNumber,
    const lamport::Signature::Shared signature):
    ConfirmationMessage(
        equivalent,
        contractor->ownIdOnContractorSide(),
        transactionUUID),
    mSignature(signature),
    mKeyNumber(keyNumber)
{
    encrypt(contractor);
}

AuditResponseMessage::AuditResponseMessage(
    const SerializedEquivalent equivalent,
    Contractor::Shared contractor,
    const TransactionUUID &transactionUUID,
    OperationState state) :
    ConfirmationMessage(
        equivalent,
        contractor->ownIdOnContractorSide(),
        transactionUUID,
        state),
    mSignature(nullptr)
{
    encrypt(contractor);
}

AuditResponseMessage::AuditResponseMessage(
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

const Message::MessageType AuditResponseMessage::typeID() const
{
    return Message::TrustLines_AuditConfirmation;
}

const uint32_t AuditResponseMessage::keyNumber() const
{
    return mKeyNumber;
}

const lamport::Signature::Shared AuditResponseMessage::signature() const
{
    return mSignature;
}

pair<BytesShared, size_t> AuditResponseMessage::serializeToBytes() const
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

    if (state() == ConfirmationMessage::OK) {
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
    }

    return make_pair(
        buffer,
        kBufferSize);
}
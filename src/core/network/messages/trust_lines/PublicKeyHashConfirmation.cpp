#include "PublicKeyHashConfirmation.h"

PublicKeyHashConfirmation::PublicKeyHashConfirmation(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    KeyNumber number,
    lamport::KeyHash::Shared hashConfirmation):
    ConfirmationMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mNumber(number),
    mHashConfirmation(hashConfirmation)
{}

PublicKeyHashConfirmation::PublicKeyHashConfirmation(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    OperationState state) :
    ConfirmationMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        state),
    mNumber(0),
    mHashConfirmation(nullptr)
{}

PublicKeyHashConfirmation::PublicKeyHashConfirmation(
    BytesShared buffer) :
    ConfirmationMessage(buffer)
{
    auto bytesBufferOffset = ConfirmationMessage::kOffsetToInheritedBytes();

    if (state() == ConfirmationMessage::OK) {
        memcpy(
            &mNumber,
            buffer.get() + bytesBufferOffset,
            sizeof(KeyNumber));
        bytesBufferOffset += sizeof(KeyNumber);

        mHashConfirmation = make_shared<lamport::KeyHash>(
            buffer.get() + bytesBufferOffset);
    }
}

const Message::MessageType PublicKeyHashConfirmation::typeID() const
{
    return Message::TrustLines_HashConfirmation;
}

const KeyNumber PublicKeyHashConfirmation::number() const
{
    return mNumber;
}

const lamport::KeyHash::Shared PublicKeyHashConfirmation::hashConfirmation() const
{
    return mHashConfirmation;
}

pair<BytesShared, size_t> PublicKeyHashConfirmation::serializeToBytes() const
    throw (bad_alloc)
{
    const auto parentBytesAndCount = ConfirmationMessage::serializeToBytes();
    auto kBufferSize = parentBytesAndCount.second;
    if (state() == ConfirmationMessage::OK) {
        kBufferSize += sizeof(KeyNumber) + lamport::KeyHash::kBytesSize;
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
            &mNumber,
            sizeof(KeyNumber));
        dataBytesOffset += sizeof(KeyNumber);

        memcpy(
            buffer.get() + dataBytesOffset,
            mHashConfirmation->data(),
            lamport::KeyHash::kBytesSize);
    }

    return make_pair(
        buffer,
        kBufferSize);
}
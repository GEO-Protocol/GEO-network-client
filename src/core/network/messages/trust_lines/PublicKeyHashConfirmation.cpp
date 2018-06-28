#include "PublicKeyHashConfirmation.h"

PublicKeyHashConfirmation::PublicKeyHashConfirmation(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    KeyNumber number,
    lamport::KeyHash::Shared hashConfirmation):
    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mNumber(number),
    mHashConfirmation(hashConfirmation)
{}

PublicKeyHashConfirmation::PublicKeyHashConfirmation(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    memcpy(
        &mNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(KeyNumber));
    bytesBufferOffset += sizeof(KeyNumber);

    mHashConfirmation = make_shared<lamport::KeyHash>(
        buffer.get() + bytesBufferOffset);
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
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(KeyNumber)
            + lamport::KeyHash::kBytesSize;
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
        sizeof(KeyNumber));
    dataBytesOffset += sizeof(KeyNumber);

    memcpy(
        buffer.get() + dataBytesOffset,
        mHashConfirmation->data(),
        lamport::KeyHash::kBytesSize);

    return make_pair(
        buffer,
        kBufferSize);
}
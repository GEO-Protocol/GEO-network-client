#include "PublicKeyMessage.h"

PublicKeyMessage::PublicKeyMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const KeyNumber number,
    const lamport::PublicKey::Shared publicKey):
    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mNumber(number),
    mPublicKey(publicKey)
{}

PublicKeyMessage::PublicKeyMessage(
    BytesShared buffer) :
    TransactionMessage(buffer),
    mPublicKey()
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    memcpy(
        &mNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(KeyNumber));
    bytesBufferOffset += sizeof(KeyNumber);

    auto publicKey = make_shared<lamport::PublicKey>(
        buffer.get() + bytesBufferOffset);
    mPublicKey = publicKey;
}

const Message::MessageType PublicKeyMessage::typeID() const
{
    return Message::TrustLines_PublicKey;
}

const KeyNumber PublicKeyMessage::number() const
{
    return mNumber;
}

const lamport::PublicKey::Shared PublicKeyMessage::publicKey() const
{
    return mPublicKey;
}

const bool PublicKeyMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}

const bool PublicKeyMessage::isCheckCachedResponse() const
{
    return true;
}

pair<BytesShared, size_t> PublicKeyMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(KeyNumber)
            + mPublicKey->keySize();
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
        mPublicKey->data(),
        mPublicKey->keySize());

    return make_pair(
        buffer,
        kBufferSize);
}
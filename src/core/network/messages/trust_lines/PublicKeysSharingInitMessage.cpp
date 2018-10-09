#include "PublicKeysSharingInitMessage.h"

PublicKeysSharingInitMessage::PublicKeysSharingInitMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const KeysCount keysCount,
    const KeyNumber number,
    const lamport::PublicKey::Shared publicKey):
    PublicKeyMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        number,
        publicKey),
    mKeysCount(keysCount)
{}

PublicKeysSharingInitMessage::PublicKeysSharingInitMessage(
    BytesShared buffer) :
    PublicKeyMessage(buffer)
{
    auto bytesBufferOffset = PublicKeyMessage::kOffsetToInheritedBytes();

    memcpy(
        &mKeysCount,
        buffer.get() + bytesBufferOffset,
        sizeof(KeysCount));
}

const Message::MessageType PublicKeysSharingInitMessage::typeID() const
{
    return Message::TrustLines_PublicKeysSharingInit;
}

const KeyNumber PublicKeysSharingInitMessage::keysCount() const
{
    return mKeysCount;
}

const bool PublicKeysSharingInitMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}

const bool PublicKeysSharingInitMessage::isCheckCachedResponse() const
{
    return true;
}

pair<BytesShared, size_t> PublicKeysSharingInitMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = PublicKeyMessage::serializeToBytes();
    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(KeysCount);
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
        &mKeysCount,
        sizeof(KeysCount));

    return make_pair(
        buffer,
        kBufferSize);
}
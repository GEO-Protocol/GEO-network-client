#include "PublicKeyMessage.h"

PublicKeyMessage::PublicKeyMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    uint32_t number,
    const CryptoKey &publicKey):
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
        sizeof(uint32_t));
    bytesBufferOffset += sizeof(uint32_t);
    size_t publicKeySize;
    memcpy(
        &publicKeySize,
        buffer.get() + bytesBufferOffset,
        sizeof(size_t));
    bytesBufferOffset += sizeof(size_t);

    mPublicKey.deserialize(
        publicKeySize,
        buffer.get() + bytesBufferOffset);
}

const Message::MessageType PublicKeyMessage::typeID() const
{
    return Message::TrustLines_PublicKey;
}

const uint32_t PublicKeyMessage::number() const
{
    return mNumber;
}

const CryptoKey& PublicKeyMessage::publicKey() const
{
    return mPublicKey;
}

pair<BytesShared, size_t> PublicKeyMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(uint32_t)
            + sizeof(size_t)
            + mPublicKey.keySize();
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

    auto publicKeySize = mPublicKey.keySize();
    memcpy(
        buffer.get() + dataBytesOffset,
        &publicKeySize,
        sizeof(size_t));
    dataBytesOffset += sizeof(size_t);

    memcpy(
        buffer.get() + dataBytesOffset,
        mPublicKey.key(),
        publicKeySize);

    return make_pair(
        buffer,
        kBufferSize);
}
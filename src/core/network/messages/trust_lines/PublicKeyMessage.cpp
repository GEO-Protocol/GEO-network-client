#include "PublicKeyMessage.h"

PublicKeyMessage::PublicKeyMessage(
    const SerializedEquivalent equivalent,
    Contractor::Shared contractor,
    const TransactionUUID &transactionUUID,
    const KeyNumber number,
    const lamport::PublicKey::Shared publicKey):
    TransactionMessage(
        equivalent,
        contractor->ownIdOnContractorSide(),
        transactionUUID),
    mNumber(number),
    mPublicKey(publicKey)
{
    encrypt(contractor);
}

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

const size_t PublicKeyMessage::kOffsetToInheritedBytes() const
{
    const auto kOffset =
            TransactionMessage::kOffsetToInheritedBytes()
            + sizeof(KeyNumber)
            + mPublicKey->keySize();
    return kOffset;
}
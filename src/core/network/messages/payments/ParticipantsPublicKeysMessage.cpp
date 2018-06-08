#include "ParticipantsPublicKeysMessage.h"

ParticipantsPublicKeysMessage::ParticipantsPublicKeysMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const map<PaymentNodeID, CryptoKey> &publicKeys):
    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mPublicKeys(publicKeys)
{}

ParticipantsPublicKeysMessage::ParticipantsPublicKeysMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    SerializedRecordsCount kRecordsCount;
    memcpy(
        &kRecordsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber i = 0; i < kRecordsCount; ++i) {
        PaymentNodeID paymentNodeID;
        memcpy(
            &paymentNodeID,
            buffer.get() + bytesBufferOffset,
            sizeof(PaymentNodeID));
        bytesBufferOffset += sizeof(PaymentNodeID);

        size_t publicKeySize;
        memcpy(
            &publicKeySize,
            buffer.get() + bytesBufferOffset,
            sizeof(size_t));
        bytesBufferOffset += sizeof(size_t);

        CryptoKey publicKey(
            buffer.get() + bytesBufferOffset,
            publicKeySize);
        bytesBufferOffset += publicKeySize;

        mPublicKeys.insert(
            make_pair(
                paymentNodeID,
                publicKey));
    }
}

const Message::MessageType ParticipantsPublicKeysMessage::typeID() const
{
    return Message::Payments_ParticipantsPublicKeys;
}

const map<PaymentNodeID, CryptoKey>& ParticipantsPublicKeysMessage::publicKeys() const
{
    return mPublicKeys;
}

pair<BytesShared, size_t> ParticipantsPublicKeysMessage::serializeToBytes() const
    throw(bad_alloc)
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(SerializedRecordsCount)
            + mPublicKeys.size()
              * (sizeof(PaymentNodeID) * CryptoKey::serializedKeySize());

    BytesShared buffer = tryMalloc(kBufferSize);

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    // Records count
    auto kTotalParticipantsCount = mPublicKeys.size();
    memcpy(
        buffer.get() + dataBytesOffset,
        &kTotalParticipantsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    // Nodes IDs and publicKeys
    for (const auto &nodeIDAndPublicKey : mPublicKeys) {
        memcpy(
            buffer.get() + dataBytesOffset,
            &nodeIDAndPublicKey.first,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);

        auto publicKeySize = nodeIDAndPublicKey.second.keySize();
        memcpy(
            buffer.get() + dataBytesOffset,
            &publicKeySize,
            sizeof(size_t));
        dataBytesOffset += sizeof(size_t);

        memcpy(
            buffer.get() + dataBytesOffset,
            nodeIDAndPublicKey.second.key(),
            publicKeySize);
        dataBytesOffset += publicKeySize;
    }

    return make_pair(
        buffer,
        kBufferSize);
}
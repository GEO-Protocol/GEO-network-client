#include "ObservingRequestMessage.h"

ObservingRequestMessage::ObservingRequestMessage(
    const TransactionUUID &transactionUUID,
    const map<PaymentNodeID, lamport::PublicKey::Shared> &participantsPublicKeys):
    mTransactionUUID(transactionUUID),
    mParticipantsPublicKeys(participantsPublicKeys)
{}

ObservingRequestMessage::ObservingRequestMessage(
    BytesShared buffer)
{
    auto bytesBufferOffset = Message::kOffsetToInheritedBytes();

    memcpy(
        mTransactionUUID.data,
        buffer.get() + bytesBufferOffset,
        TransactionUUID::kBytesSize);
    bytesBufferOffset += TransactionUUID::kBytesSize;

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

        auto publicKey = make_shared<lamport::PublicKey>(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += lamport::PublicKey::keySize();

        mParticipantsPublicKeys.insert(
            make_pair(
                paymentNodeID,
                publicKey));
    }
}

const Message::MessageType ObservingRequestMessage::typeID() const
{
    return Message::Observing_Request;
}

pair<BytesShared, size_t> ObservingRequestMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = Message::serializeToBytes();

    const auto kBufferSize =
            parentBytesAndCount.second
            + TransactionUUID::kBytesSize
            + sizeof(SerializedRecordsCount)
            + mParticipantsPublicKeys.size()
              * (sizeof(PaymentNodeID) + lamport::PublicKey::keySize());
    // todo : add hash size

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
        mTransactionUUID.data,
        TransactionUUID::kBytesSize);
    dataBytesOffset += TransactionUUID::kBytesSize;

    // Records count
    auto kTotalParticipantsCount = mParticipantsPublicKeys.size();
    memcpy(
        buffer.get() + dataBytesOffset,
        &kTotalParticipantsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    // Nodes IDs and publicKeys
    for (const auto &nodeIDAndPublicKey : mParticipantsPublicKeys) {
        memcpy(
            buffer.get() + dataBytesOffset,
            &nodeIDAndPublicKey.first,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);

        memcpy(
            buffer.get() + dataBytesOffset,
            nodeIDAndPublicKey.second->data(),
            lamport::PublicKey::keySize());
        dataBytesOffset += lamport::PublicKey::keySize();
    }

    // todo : add hash of all data

    return make_pair(
        buffer,
        kBufferSize);
}
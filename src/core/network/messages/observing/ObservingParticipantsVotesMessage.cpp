#include "ObservingParticipantsVotesMessage.h"

ObservingParticipantsVotesMessage::ObservingParticipantsVotesMessage(
    const TransactionUUID& transactionUUID,
    const map<PaymentNodeID, lamport::Signature::Shared> &participantsSignatures) :
    mTransactionUUID(transactionUUID),
    mParticipantsSignatures(participantsSignatures)
{}

ObservingParticipantsVotesMessage::ObservingParticipantsVotesMessage(
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
        auto *paymentNodeID = new (buffer.get() + bytesBufferOffset) PaymentNodeID;
        bytesBufferOffset += sizeof(PaymentNodeID);

        auto signature = make_shared<lamport::Signature>(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += lamport::Signature::signatureSize();

        mParticipantsSignatures.insert(
            make_pair(
                *paymentNodeID,
                signature));
    }
}

const Message::MessageType ObservingParticipantsVotesMessage::typeID() const
{
    return Message::Observing_ParticipantsVotes;
}

pair<BytesShared, size_t> ObservingParticipantsVotesMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = Message::serializeToBytes();

    const auto kTotalParticipantsCount = mParticipantsSignatures.size();

    const auto kBufferSize =
            parentBytesAndCount.second
            + TransactionUUID::kBytesSize
            + sizeof(SerializedRecordsCount)
            + kTotalParticipantsCount
              * (sizeof(PaymentNodeID) + lamport::Signature::signatureSize());

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
    memcpy(
        buffer.get() + dataBytesOffset,
        &kTotalParticipantsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    // Payment Node IDs and signatures
    for (const auto &paymentNodeIDAndVote : mParticipantsSignatures) {

        const auto kParticipantPaymentID = paymentNodeIDAndVote.first;
        memcpy(
            buffer.get() + dataBytesOffset,
            &kParticipantPaymentID,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);

        memcpy(
            buffer.get() + dataBytesOffset,
            paymentNodeIDAndVote.second->data(),
            lamport::Signature::signatureSize());
        dataBytesOffset += lamport::Signature::signatureSize();
    }

    return make_pair(
        buffer,
        kBufferSize);
}

const TransactionUUID& ObservingParticipantsVotesMessage::transactionUUID() const
{
    return mTransactionUUID;
}

const map<PaymentNodeID, lamport::Signature::Shared>& ObservingParticipantsVotesMessage::participantsSignatures() const
{
    return mParticipantsSignatures;
}

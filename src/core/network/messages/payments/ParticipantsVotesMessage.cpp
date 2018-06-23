#include "ParticipantsVotesMessage.h"

ParticipantsVotesMessage::ParticipantsVotesMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    map<PaymentNodeID, lamport::Signature::Shared> &participantsSignatures) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mParticipantsSignatures(participantsSignatures)
{}

ParticipantsVotesMessage::ParticipantsVotesMessage(
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

const Message::MessageType ParticipantsVotesMessage::typeID() const
{
    return Message::Payments_ParticipantsVotes;
}

/**
 * Serializes the message into shared bytes sequence.
 *
 * Message format:
 *  16B - Transaction UUID,
 *  16B - Coordinator UUID,
 *  4B  - Total participants count,
 *
 *  { Participant record
 *      4B - Participant 1 PaymntID,
 *      16KB  - Participant 1 signature,
 *  }
 *
 *  ...
 *
 *  { Participant record
 *      4B - Participant N UUID
 *      16KB  - Participant N signature
 *  }
 *
 *
 *  @throws bad_alloc in case of insufficient memory.
 */
pair<BytesShared, size_t> ParticipantsVotesMessage::serializeToBytes() const
    throw(bad_alloc)
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kTotalParticipantsCount = mParticipantsSignatures.size();

    const auto kBufferSize =
        parentBytesAndCount.second
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

const map<PaymentNodeID, lamport::Signature::Shared>& ParticipantsVotesMessage::participantsSignatures() const
{
    return mParticipantsSignatures;
}

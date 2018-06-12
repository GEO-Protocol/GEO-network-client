#include "ParticipantsVotesMessage.h"

ParticipantsVotesMessage::ParticipantsVotesMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    map<PaymentNodeID, BytesShared> &participantsSigns) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mParticipantsSigns(participantsSigns)
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

        BytesShared sign = tryMalloc(4);
        memcpy(
            sign.get(),
            buffer.get() + bytesBufferOffset,
            4);

        mParticipantsSigns.insert(
            make_pair(
                *paymentNodeID,
                sign));
        bytesBufferOffset += 4;
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
 *      56B  - Participant 1 vote (true/false),
 *  }
 *
 *  ...
 *
 *  { Participant record
 *      4B - Participant N UUID
 *      56B  - Participant N vote (true/false)
 *  }
 *
 *
 *  @throws bad_alloc in case of insufficient memory.
 */
pair<BytesShared, size_t> ParticipantsVotesMessage::serializeToBytes() const
    throw(bad_alloc)
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kTotalParticipantsCount = mParticipantsSigns.size();

    const auto kBufferSize =
        parentBytesAndCount.second
        + sizeof(SerializedRecordsCount)
        + kTotalParticipantsCount * (sizeof(PaymentNodeID), 4);

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

    // Nodes UUIDs and votes
    for (const auto &paymentNodeIDAndVote : mParticipantsSigns) {

        const auto kParticipantPaymentID = paymentNodeIDAndVote.first;
        memcpy(
            buffer.get() + dataBytesOffset,
            &kParticipantPaymentID,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);

        memcpy(
            buffer.get() + dataBytesOffset,
            paymentNodeIDAndVote.second.get(),
            4);
        dataBytesOffset += 4;
    }

    return make_pair(
        buffer,
        kBufferSize);
}

const map<PaymentNodeID, BytesShared>& ParticipantsVotesMessage::participantsSigns() const
{
    return mParticipantsSigns;
}

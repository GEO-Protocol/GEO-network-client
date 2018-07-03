#include "ParticipantVoteMessage.h"

ParticipantVoteMessage::ParticipantVoteMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    lamport::Signature::Shared signature) :
    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mSignature(signature)
{}

ParticipantVoteMessage::ParticipantVoteMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    auto signature = make_shared<lamport::Signature>(
        buffer.get() + bytesBufferOffset);
    mSignature = signature;
}

const Message::MessageType ParticipantVoteMessage::typeID() const
{
    return Message::Payments_ParticipantVote;
}

const lamport::Signature::Shared ParticipantVoteMessage::signature() const
{
    return mSignature;
}

pair<BytesShared, size_t> ParticipantVoteMessage::serializeToBytes() const
    throw(bad_alloc)
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kBufferSize =
            parentBytesAndCount.second
            + lamport::Signature::signatureSize();

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
        mSignature->data(),
        lamport::Signature::signatureSize());

    return make_pair(
        buffer,
        kBufferSize);
}
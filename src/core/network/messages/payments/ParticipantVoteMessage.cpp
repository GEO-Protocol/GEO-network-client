#include "ParticipantVoteMessage.h"

ParticipantVoteMessage::ParticipantVoteMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    lamport::Signature::Shared sign) :
    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mSign(sign)
{}

ParticipantVoteMessage::ParticipantVoteMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    auto sign = make_shared<lamport::Signature>(
        buffer.get() + bytesBufferOffset);
    mSign = sign;
}

const Message::MessageType ParticipantVoteMessage::typeID() const
{
    return Message::Payments_ParticipantVote;
}

const lamport::Signature::Shared ParticipantVoteMessage::sign() const
{
    return mSign;
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
        mSign->data(),
        lamport::Signature::signatureSize());

    return make_pair(
        buffer,
        kBufferSize);
}
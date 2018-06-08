#include "ParticipantVoteMessage.h"

ParticipantVoteMessage::ParticipantVoteMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    size_t signBytesCount,
    BytesShared sign) :
    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mSign(sign),
    mSignBytesCount(signBytesCount)
{}

ParticipantVoteMessage::ParticipantVoteMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    memcpy(
        &mSignBytesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(size_t));
    bytesBufferOffset += sizeof(size_t);

    memcpy(
        mSign.get(),
        buffer.get() + bytesBufferOffset,
        mSignBytesCount);
}

const Message::MessageType ParticipantVoteMessage::typeID() const
{
    return Message::Payments_ParticipantVote;
}

const pair<BytesShared, size_t> ParticipantVoteMessage::sign() const
{
    return make_pair(
        mSign,
        mSignBytesCount);
}

pair<BytesShared, size_t> ParticipantVoteMessage::serializeToBytes() const
    throw(bad_alloc)
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(size_t)
            + mSignBytesCount;

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
        &mSignBytesCount,
        sizeof(size_t));
    dataBytesOffset += sizeof(size_t);

    memcpy(
        buffer.get() + dataBytesOffset,
        mSign.get(),
        mSignBytesCount);

    return make_pair(
        buffer,
        kBufferSize);
}
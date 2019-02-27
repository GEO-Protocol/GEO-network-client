#include "ParticipantVoteMessage.h"

ParticipantVoteMessage::ParticipantVoteMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    lamport::Signature::Shared signature) :
    TransactionMessage(
        equivalent,
        senderAddresses,
        transactionUUID),
    mSignature(signature)
{}

ParticipantVoteMessage::ParticipantVoteMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    auto *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
    if (mState == Accepted) {
        bytesBufferOffset += sizeof(SerializedOperationState);
        auto signature = make_shared<lamport::Signature>(
            buffer.get() + bytesBufferOffset);
        mSignature = signature;
    }
}

const Message::MessageType ParticipantVoteMessage::typeID() const
{
    return Message::Payments_ParticipantVote;
}

const ParticipantVoteMessage::OperationState ParticipantVoteMessage::state() const
{
    return mState;
}

const lamport::Signature::Shared ParticipantVoteMessage::signature() const
{
    return mSignature;
}

pair<BytesShared, size_t> ParticipantVoteMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    auto bufferSize =
            parentBytesAndCount.second
            + sizeof(SerializedOperationState);
    if (mSignature != nullptr) {
        bufferSize += lamport::Signature::signatureSize();
    }

    BytesShared buffer = tryMalloc(bufferSize);

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    if (mSignature != nullptr) {
        SerializedOperationState state(ParticipantVoteMessage::Accepted);
        memcpy(
            buffer.get() + dataBytesOffset,
            &state,
            sizeof(SerializedOperationState));
        dataBytesOffset += sizeof(SerializedOperationState);

        memcpy(
            buffer.get() + dataBytesOffset,
            mSignature->data(),
            lamport::Signature::signatureSize());
    } else {
        SerializedOperationState state(ParticipantVoteMessage::Rejected);
        memcpy(
            buffer.get() + dataBytesOffset,
            &state,
            sizeof(SerializedOperationState));
    }

    return make_pair(
        buffer,
        bufferSize);
}
#include "ObservingParticipantsVotesRequestMessage.h"

ObservingParticipantsVotesRequestMessage::ObservingParticipantsVotesRequestMessage(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber):
    mTransactionUUID(transactionUUID),
    mMaximalClaimingBlockNumber(maximalClaimingBlockNumber)
{}

const ObservingMessage::MessageType ObservingParticipantsVotesRequestMessage::typeID() const
{
    return ObservingMessage::Observing_ParticipantsVotesRequest;
}

BytesShared ObservingParticipantsVotesRequestMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = ObservingMessage::serializeToBytes();

    BytesShared buffer = tryMalloc(serializedSize());

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.get(),
        ObservingMessage::serializedSize());
    dataBytesOffset += ObservingMessage::serializedSize();

    memcpy(
        buffer.get() + dataBytesOffset,
        &mMaximalClaimingBlockNumber,
        sizeof(BlockNumber));
    dataBytesOffset += sizeof(BlockNumber);

    memcpy(
        buffer.get() + dataBytesOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize);

    return buffer;
}

size_t ObservingParticipantsVotesRequestMessage::serializedSize() const
{
    return ObservingMessage::serializedSize()
           + TransactionUUID::kBytesSize
           + sizeof(BlockNumber);
}
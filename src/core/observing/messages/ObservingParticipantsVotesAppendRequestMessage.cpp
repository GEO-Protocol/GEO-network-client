#include "ObservingParticipantsVotesAppendRequestMessage.h"

ObservingParticipantsVotesAppendRequestMessage::ObservingParticipantsVotesAppendRequestMessage(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber,
    map<PaymentNodeID, lamport::Signature::Shared> participantsSignatures) :
    mTransactionUUID(transactionUUID),
    mMaximalClaimingBlockNumber(maximalClaimingBlockNumber),
    mParticipantsSignatures(participantsSignatures)
{}

const ObservingMessage::MessageType ObservingParticipantsVotesAppendRequestMessage::typeID() const
{
    return ObservingMessage::Observing_ParticipantsVotesAppendRequest;
}

BytesShared ObservingParticipantsVotesAppendRequestMessage::serializeToBytes() const
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
    dataBytesOffset += TransactionUUID::kBytesSize;

    // Records count
    auto kTotalParticipantsCount = mParticipantsSignatures.size();
    memcpy(
        buffer.get() + dataBytesOffset,
        &kTotalParticipantsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    // Nodes IDs and publicKeys
    for (const auto &nodeIDAndSignature : mParticipantsSignatures) {
        memcpy(
            buffer.get() + dataBytesOffset,
            &nodeIDAndSignature.first,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);

        memcpy(
            buffer.get() + dataBytesOffset,
            nodeIDAndSignature.second->data(),
            lamport::Signature::signatureSize());
        dataBytesOffset += lamport::Signature::signatureSize();
    }

    // todo : add hash of all data

    return buffer;
}

size_t ObservingParticipantsVotesAppendRequestMessage::serializedSize() const
{
    return ObservingMessage::serializedSize()
           + sizeof(BlockNumber)
           + TransactionUUID::kBytesSize
           + sizeof(SerializedRecordsCount)
           + mParticipantsSignatures.size()
             * (sizeof(PaymentNodeID) + lamport::Signature::signatureSize());
    // todo : add hash size
}

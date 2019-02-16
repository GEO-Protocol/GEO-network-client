#include "ObservingParticipantsVotesResponseMessage.h"

ObservingParticipantsVotesResponseMessage::ObservingParticipantsVotesResponseMessage(
    BytesShared buffer):
    ObservingResponseMessage(
        buffer)
{
    size_t bytesBufferOffset = ObservingResponseMessage::kOffsetToInheritedBytes();

    memcpy(
        &mIsParticipantsVotesPresent,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
    bytesBufferOffset += sizeof(byte);

    memcpy(
        &mMaximalClaimingBlockNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(BlockNumber));
    bytesBufferOffset += sizeof(BlockNumber);

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

bool ObservingParticipantsVotesResponseMessage::isParticipantsVotesPresent() const
{
    return mIsParticipantsVotesPresent;
}

const TransactionUUID& ObservingParticipantsVotesResponseMessage::transactionUUID() const
{
    return mTransactionUUID;
}

const BlockNumber ObservingParticipantsVotesResponseMessage::maximalClaimingBlockNumber() const
{
    return mMaximalClaimingBlockNumber;
}

const map<PaymentNodeID, lamport::Signature::Shared>& ObservingParticipantsVotesResponseMessage::participantsSignatures() const
{
    return mParticipantsSignatures;
}

#include "ObservingParticipantsVotesResponseMessage.h"

ObservingParticipantsVotesResponseMessage::ObservingParticipantsVotesResponseMessage(
    BytesShared buffer)
{
    size_t bytesBufferOffset = 0;

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

const ObservingMessage::MessageType ObservingParticipantsVotesResponseMessage::typeID() const
{
    return ObservingMessage::Observing_ParticipantsVotesResponse;
}

const TransactionUUID& ObservingParticipantsVotesResponseMessage::transactionUUID() const
{
    return mTransactionUUID;
}

const map<PaymentNodeID, lamport::Signature::Shared>& ObservingParticipantsVotesResponseMessage::participantsSignatures() const
{
    return mParticipantsSignatures;
}

#include "ObservingClaimAppendRequestMessage.h"

ObservingClaimAppendRequestMessage::ObservingClaimAppendRequestMessage(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber,
    const map<PaymentNodeID, lamport::PublicKey::Shared> &participantsPublicKeys):
    mTransactionUUID(transactionUUID),
    mMaximalClaimingBlockNumber(maximalClaimingBlockNumber),
    mParticipantsPublicKeys(participantsPublicKeys)
{}

const ObservingMessage::MessageType ObservingClaimAppendRequestMessage::typeID() const
{
    return ObservingMessage::Observing_ClaimAppendRequest;
}

const TransactionUUID& ObservingClaimAppendRequestMessage::transactionUUID() const
{
    return mTransactionUUID;
}

const BlockNumber ObservingClaimAppendRequestMessage::maximalClaimingBlockNumber() const
{
    return mMaximalClaimingBlockNumber;
}

BytesShared ObservingClaimAppendRequestMessage::serializeToBytes() const
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
    auto kTotalParticipantsCount = mParticipantsPublicKeys.size();
    memcpy(
        buffer.get() + dataBytesOffset,
        &kTotalParticipantsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    // Nodes IDs and publicKeys
    for (const auto &nodeIDAndPublicKey : mParticipantsPublicKeys) {
        memcpy(
            buffer.get() + dataBytesOffset,
            &nodeIDAndPublicKey.first,
            sizeof(PaymentNodeID));
        dataBytesOffset += sizeof(PaymentNodeID);

        memcpy(
            buffer.get() + dataBytesOffset,
            nodeIDAndPublicKey.second->data(),
            lamport::PublicKey::keySize());
        dataBytesOffset += lamport::PublicKey::keySize();
    }

    // todo : add hash of all data

    return buffer;
}

size_t ObservingClaimAppendRequestMessage::serializedSize() const
{
    return ObservingMessage::serializedSize()
           + sizeof(BlockNumber)
           + TransactionUUID::kBytesSize
           + sizeof(SerializedRecordsCount)
           + mParticipantsPublicKeys.size()
             * (sizeof(PaymentNodeID) + lamport::PublicKey::keySize());
    // todo : add hash size
}
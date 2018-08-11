#include "ReceiptRecord.h"

ReceiptRecord::ReceiptRecord(
    const AuditNumber auditNumber,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const lamport::KeyHash::Shared keyHash,
    const lamport::Signature::Shared signature) :
    mAuditNumber(auditNumber),
    mTransactionUUID(transactionUUID),
    mAmount(amount),
    mKeyHash(keyHash),
    mSignature(signature)
{}

ReceiptRecord::ReceiptRecord(
    byte* buffer)
{
    auto bytesBufferOffset = 0;
    memcpy(
        &mAuditNumber,
        buffer + bytesBufferOffset,
        sizeof(AuditNumber));
    bytesBufferOffset += sizeof(AuditNumber);

    memcpy(
        mTransactionUUID.data,
        buffer + bytesBufferOffset,
        TransactionUUID::kBytesSize);
    bytesBufferOffset += TransactionUUID::kBytesSize;

    vector<byte> amountBytes(
        buffer + bytesBufferOffset,
        buffer + bytesBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(amountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    mKeyHash = make_shared<lamport::KeyHash>(
        buffer + bytesBufferOffset);
    bytesBufferOffset += lamport::KeyHash::kBytesSize;

    mSignature = make_shared<lamport::Signature>(
        buffer + bytesBufferOffset);
}

const AuditNumber ReceiptRecord::auditNumber() const
{
    return mAuditNumber;
}

const TransactionUUID& ReceiptRecord::transactionUUID() const
{
    return mTransactionUUID;
}

const TrustLineAmount& ReceiptRecord::amount() const
{
    return mAmount;
}

const lamport::KeyHash::Shared ReceiptRecord::keyHash() const
{
    return mKeyHash;
}

const lamport::Signature::Shared ReceiptRecord::signature() const
{
    return mSignature;
}

BytesShared ReceiptRecord::serializeToBytes()
{
    BytesShared dataBytesShared = tryCalloc(recordSize());
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize);
    dataBytesOffset += TransactionUUID::kBytesSize;

    vector<byte> amountBufferBytes = trustLineAmountToBytes(
        mAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        amountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mKeyHash->data(),
        lamport::KeyHash::kBytesSize);
    dataBytesOffset += lamport::KeyHash::kBytesSize;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mSignature->data(),
        lamport::Signature::signatureSize());

    return dataBytesShared;
}

const size_t ReceiptRecord::recordSize()
{
    return sizeof(AuditNumber)
           + TransactionUUID::kBytesSize
           + kTrustLineAmountBytesCount
           + lamport::KeyHash::kBytesSize
           + lamport::Signature::signatureSize();
}

bool operator== (
    const ReceiptRecord::Shared record1,
    const ReceiptRecord::Shared record2)
{
    if (record1->auditNumber() != record2->auditNumber()) {
        return false;
    }
    if (record1->transactionUUID() != record2->transactionUUID()) {
        return false;
    }
    if (record1->amount() != record2->amount()) {
        return false;
    }
    if (*record1->keyHash() != *record2->keyHash()) {
        return false;
    }
    // todo compare signatures
    return true;
}
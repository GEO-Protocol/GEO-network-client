#include "AuditRecord.h"

AuditRecord::AuditRecord(
    AuditNumber auditNumber,
    TrustLineAmount &incomingAmount,
    TrustLineAmount &outgoingAmount,
    TrustLineBalance &balance) :

    mAuditNumber(auditNumber),
    mIncomingAmount(incomingAmount),
    mOutgoingAmount(outgoingAmount),
    mBalance(balance),
    mOwnKeyHash(nullptr),
    mOwnSignature(nullptr),
    mContractorKeyHash(nullptr),
    mContractorSignature(nullptr)
{}

AuditRecord::AuditRecord(
    AuditNumber auditNumber,
    TrustLineAmount &incomingAmount,
    TrustLineAmount &outgoingAmount,
    TrustLineBalance &balance,
    lamport::KeyHash::Shared ownKeyHash,
    lamport::Signature::Shared ownSignature,
    lamport::KeyHash::Shared contractorKeyHash,
    lamport::Signature::Shared contractorSignature):

    mAuditNumber(auditNumber),
    mIncomingAmount(incomingAmount),
    mOutgoingAmount(outgoingAmount),
    mBalance(balance),
    mOwnKeyHash(ownKeyHash),
    mOwnSignature(ownSignature),
    mContractorKeyHash(contractorKeyHash),
    mContractorSignature(contractorSignature)
{}

AuditRecord::AuditRecord(
    byte* buffer)
{
    auto bytesBufferOffset = 0;
    memcpy(
        &mAuditNumber,
        buffer + bytesBufferOffset,
        sizeof(AuditNumber));
    bytesBufferOffset += sizeof(AuditNumber);

    vector<byte> incomingAmountBytes(
        buffer + bytesBufferOffset,
        buffer + bytesBufferOffset + kTrustLineAmountBytesCount);
    mIncomingAmount = bytesToTrustLineAmount(incomingAmountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> outgoingAmountBytes(
        buffer + bytesBufferOffset,
        buffer + bytesBufferOffset + kTrustLineAmountBytesCount);
    mOutgoingAmount = bytesToTrustLineAmount(outgoingAmountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBytes(
        buffer + bytesBufferOffset,
        buffer + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);
    mBalance = bytesToTrustLineBalance(balanceBytes);
    bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;

    mOwnKeyHash = make_shared<lamport::KeyHash>(
        buffer + bytesBufferOffset);
    bytesBufferOffset += lamport::KeyHash::kBytesSize;

    mOwnSignature = make_shared<lamport::Signature>(
        buffer + bytesBufferOffset);
    bytesBufferOffset += lamport::Signature::signatureSize();

    mContractorKeyHash = make_shared<lamport::KeyHash>(
        buffer + bytesBufferOffset);
    bytesBufferOffset += lamport::KeyHash::kBytesSize;

    mContractorSignature = make_shared<lamport::Signature>(
        buffer + bytesBufferOffset);
}

const AuditNumber AuditRecord::auditNumber() const
{
    return mAuditNumber;
}

const TrustLineAmount& AuditRecord::incomingAmount() const
{
    return mIncomingAmount;
}

const TrustLineAmount& AuditRecord::outgoingAmount() const
{
    return mOutgoingAmount;
}

const TrustLineBalance& AuditRecord::balance() const
{
    return mBalance;
}

const lamport::KeyHash::Shared AuditRecord::ownKeyHash() const
{
    return mOwnKeyHash;
}

const lamport::Signature::Shared AuditRecord::ownSignature() const
{
    return mOwnSignature;
}

const lamport::KeyHash::Shared AuditRecord::contractorKeyHash() const
{
    return mContractorKeyHash;
}

const lamport::Signature::Shared AuditRecord::contractorSignature() const
{
    return mContractorSignature;
}

BytesShared AuditRecord::serializeToBytes()
{
    BytesShared dataBytesShared = tryCalloc(recordSize());
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mIncomingAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mOutgoingAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(mBalance));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);
    dataBytesOffset += kTrustLineBalanceSerializeBytesCount;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mOwnKeyHash->data(),
        lamport::KeyHash::kBytesSize);
    dataBytesOffset += lamport::KeyHash::kBytesSize;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mOwnSignature->data(),
        lamport::Signature::signatureSize());
    dataBytesOffset += lamport::Signature::signatureSize();

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mContractorKeyHash->data(),
        lamport::KeyHash::kBytesSize);
    dataBytesOffset += lamport::KeyHash::kBytesSize;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mContractorKeyHash->data(),
        lamport::Signature::signatureSize());

    return dataBytesShared;
}

BytesShared AuditRecord::serializeToCheckSignatureByInitiator()
{
    BytesShared dataBytesShared = tryCalloc(recordSizeForSignatureChecking());
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mIncomingAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mOutgoingAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(mBalance));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);

    return dataBytesShared;
}

BytesShared AuditRecord::serializeToCheckSignatureByContractor()
{
    BytesShared dataBytesShared = tryCalloc(recordSizeForSignatureChecking());
    size_t dataBytesOffset = 0;

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);

    vector<byte> outgoingAmountBufferBytes = trustLineAmountToBytes(
        mOutgoingAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        outgoingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    vector<byte> incomingAmountBufferBytes = trustLineAmountToBytes(
        mIncomingAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        incomingAmountBufferBytes.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;

    auto contractorBalance = -1 * mBalance;
    vector<byte> balanceBufferBytes = trustLineBalanceToBytes(
        const_cast<TrustLineBalance&>(contractorBalance));
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        balanceBufferBytes.data(),
        kTrustLineBalanceSerializeBytesCount);

    return dataBytesShared;
}

const size_t AuditRecord::recordSize()
{
    return sizeof(AuditNumber)
           + kTrustLineAmountBytesCount
           + kTrustLineAmountBytesCount
           + kTrustLineBalanceSerializeBytesCount
           + lamport::KeyHash::kBytesSize
           + lamport::Signature::signatureSize()
           + lamport::KeyHash::kBytesSize
           + lamport::Signature::signatureSize();
}

const size_t AuditRecord::recordSizeForSignatureChecking()
{
    return sizeof(AuditNumber)
           + kTrustLineAmountBytesCount
           + kTrustLineAmountBytesCount
           + kTrustLineBalanceSerializeBytesCount;
}
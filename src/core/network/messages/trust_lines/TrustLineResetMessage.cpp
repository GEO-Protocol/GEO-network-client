#include "TrustLineResetMessage.h"

TrustLineResetMessage::TrustLineResetMessage(
    const SerializedEquivalent equivalent,
    Contractor::Shared contractor,
    const TransactionUUID &transactionUUID,
    const AuditNumber auditNumber,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount,
    const TrustLineBalance &balance):
    TransactionMessage(
        equivalent,
        contractor->ownIdOnContractorSide(),
        transactionUUID),
    mAuditNumber(auditNumber),
    mIncomingAmount(incomingAmount),
    mOutgoingAmount(outgoingAmount),
    mBalance(balance)
{
    encrypt(contractor);
}

TrustLineResetMessage::TrustLineResetMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    memcpy(
        &mAuditNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(AuditNumber));
    bytesBufferOffset += sizeof(AuditNumber);

    vector<byte> incomingAmountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mIncomingAmount = bytesToTrustLineAmount(incomingAmountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> outgoingAmountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mOutgoingAmount = bytesToTrustLineAmount(outgoingAmountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balance(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);
    mBalance = bytesToTrustLineBalance(balance);
}

const Message::MessageType TrustLineResetMessage::typeID() const
{
    return Message::TrustLines_Reset;
}

const AuditNumber TrustLineResetMessage::auditNumber() const
{
    return mAuditNumber;
}

const TrustLineAmount& TrustLineResetMessage::incomingAmount() const
{
    return mIncomingAmount;
}

const TrustLineAmount& TrustLineResetMessage::outgoingAmount() const
{
    return mOutgoingAmount;
}

const TrustLineBalance& TrustLineResetMessage::balance() const
{
    return mBalance;
}

const bool TrustLineResetMessage::isCheckCachedResponse() const
{
    return true;
}

pair<BytesShared, size_t> TrustLineResetMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    auto kBufferSize = parentBytesAndCount.second
                       + sizeof(AuditNumber)
                       + kTrustLineAmountBytesCount
                       + kTrustLineAmountBytesCount
                       + kTrustLineBalanceSerializeBytesCount;
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
        &mAuditNumber,
        sizeof(AuditNumber));
    dataBytesOffset += sizeof(AuditNumber);

    vector<byte> incomingAmountBuffer = trustLineAmountToBytes(mIncomingAmount);
    memcpy(
        buffer.get() + dataBytesOffset,
        incomingAmountBuffer.data(),
        incomingAmountBuffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;

    vector<byte> outgoingAmountBuffer = trustLineAmountToBytes(mOutgoingAmount);
    memcpy(
        buffer.get() + dataBytesOffset,
        outgoingAmountBuffer.data(),
        outgoingAmountBuffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBuffer = trustLineBalanceToBytes(mBalance);
    memcpy(
        buffer.get() + dataBytesOffset,
        balanceBuffer.data(),
        balanceBuffer.size());

    return make_pair(
        buffer,
        kBufferSize);
}
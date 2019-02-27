#include "PaymentRecord.h"

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation):

    Record(
        Record::PaymentRecordType,
        operationUUID,
        contractor),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(CommandUUID::empty())
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const GEOEpochTimestamp geoEpochTimestamp):

    Record(
        Record::PaymentRecordType,
        operationUUID,
        contractor,
        geoEpochTimestamp),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(CommandUUID::empty())
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const CommandUUID &commandUUID):

    Record(
        Record::PaymentRecordType,
        operationUUID,
        contractor),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(commandUUID)
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const CommandUUID &commandUUID,
    const GEOEpochTimestamp geoEpochTimestamp):

    Record(
        Record::PaymentRecordType,
        operationUUID,
        contractor,
        geoEpochTimestamp),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(commandUUID)
{}

const bool PaymentRecord::isPaymentRecord() const
{
    return true;
}

const PaymentRecord::PaymentOperationType PaymentRecord::paymentOperationType() const
{
    return mPaymentOperationType;
}

const TrustLineAmount& PaymentRecord::amount() const
{
    return mAmount;
}

const TrustLineBalance& PaymentRecord::balanceAfterOperation() const
{
    return mBalanceAfterOperation;
}

const CommandUUID& PaymentRecord::commandUUID() const
{
    return mCommandUUID;
}

pair<BytesShared, size_t> PaymentRecord::serializedHistoryRecordBody() const
{
    size_t recordBodySize = sizeof(SerializedPaymentOperationType)
                            + mContractor->serializedSize()
                            + kTrustLineAmountBytesCount
                            + kTrustLineBalanceSerializeBytesCount;

    BytesShared bytesBuffer = tryCalloc(
        recordBodySize);
    size_t bytesBufferOffset = 0;

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &mPaymentOperationType,
        sizeof(SerializedPaymentOperationType));
    bytesBufferOffset += sizeof(SerializedPaymentOperationType);

    auto contractorSerializedData = mContractor->serializeToBytes();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        contractorSerializedData.get(),
        mContractor->serializedSize());
    bytesBufferOffset += mContractor->serializedSize();

    auto trustAmountBytes = trustLineAmountToBytes(mAmount);
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustAmountBytes.data(),
        kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    auto trustBalanceBytes = trustLineBalanceToBytes(mBalanceAfterOperation);
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustBalanceBytes.data(),
        kTrustLineBalanceSerializeBytesCount);

    return make_pair(
        bytesBuffer,
        recordBodySize);
}
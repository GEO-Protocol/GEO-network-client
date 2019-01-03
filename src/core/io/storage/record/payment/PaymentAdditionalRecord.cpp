#include "PaymentAdditionalRecord.h"

PaymentAdditionalRecord::PaymentAdditionalRecord(
    const TransactionUUID &operationUUID,
    const PaymentAdditionalOperationType operationType,
    const TrustLineAmount &amount):

    Record(
        Record::PaymentAdditionalRecordType,
        operationUUID,
        nullptr),
    mPaymentOperationType(operationType),
    mAmount(amount)
{}

PaymentAdditionalRecord::PaymentAdditionalRecord(
    const TransactionUUID &operationUUID,
    const PaymentAdditionalOperationType operationType,
    const TrustLineAmount &amount,
    const GEOEpochTimestamp geoEpochTimestamp):

    Record(
        Record::PaymentAdditionalRecordType,
        operationUUID,
        nullptr,
        geoEpochTimestamp),
    mPaymentOperationType(operationType),
    mAmount(amount)
{}

const PaymentAdditionalRecord::PaymentAdditionalOperationType PaymentAdditionalRecord::operationType() const
{
    return mPaymentOperationType;
}

const TrustLineAmount& PaymentAdditionalRecord::amount() const
{
    return mAmount;
}

pair<BytesShared, size_t> PaymentAdditionalRecord::serializedHistoryRecordBody() const
{
    size_t recordBodySize = sizeof(SerializedPaymentOperationType)
                            + kTrustLineAmountBytesCount;

    BytesShared bytesBuffer = tryCalloc(
        recordBodySize);
    size_t bytesBufferOffset = 0;

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &mPaymentOperationType,
        sizeof(SerializedPaymentOperationType));
    bytesBufferOffset += sizeof(SerializedPaymentOperationType);

    auto trustAmountBytes = trustLineAmountToBytes(mAmount);
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustAmountBytes.data(),
        kTrustLineAmountBytesCount);

    return make_pair(
        bytesBuffer,
        recordBodySize);
}
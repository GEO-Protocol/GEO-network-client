#include "PaymentRecord.h"

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    BytesShared buffer):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID)
{
    size_t dataBufferOffset = 0;

    SerializedPaymentOperationType *operationType = new (buffer.get() + dataBufferOffset) SerializedPaymentOperationType;
    mPaymentOperationType = (PaymentRecord::PaymentOperationType) *operationType;
    dataBufferOffset += sizeof(
        SerializedPaymentOperationType);

    memcpy(
        mContractorUUID.data,
        buffer.get() + dataBufferOffset,
        NodeUUID::kBytesSize);
    dataBufferOffset += NodeUUID::kBytesSize;

    vector<byte> amountBytes(
        buffer.get() + dataBufferOffset,
        buffer.get() + dataBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(
        amountBytes);
    dataBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBytes(
        buffer.get() + dataBufferOffset,
        buffer.get() + dataBufferOffset + kTrustLineBalanceBytesCount);
    mBalanceAfterOperation = bytesToTrustLineBalance(
        balanceBytes);
}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID),
    mPaymentOperationType(operationType),
    mContractorUUID(contractorUUID),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation) {}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const GEOEpochTimestamp geoEpochTimestamp):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID,
        geoEpochTimestamp),
    mPaymentOperationType(operationType),
    mContractorUUID(contractorUUID),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation) {}

const bool PaymentRecord::isPaymentRecord() const {

    return true;
}

const PaymentRecord::PaymentOperationType PaymentRecord::paymentOperationType() const {

    return mPaymentOperationType;
}

const NodeUUID PaymentRecord::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineAmount PaymentRecord::amount() const {

    return mAmount;
}

const TrustLineBalance PaymentRecord::balanceAfterOperation() const {

    return mBalanceAfterOperation;
}
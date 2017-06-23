#include "PaymentRecord.h"

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
    mBalanceAfterOperation(balanceAfterOperation)
{}

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
    mBalanceAfterOperation(balanceAfterOperation)
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const TrustLineAmount &amount):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID),
    mPaymentOperationType(operationType),
    mContractorUUID(NodeUUID::empty()),
    mAmount(amount)
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const TrustLineAmount &amount,
    const GEOEpochTimestamp geoEpochTimestamp):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID,
        geoEpochTimestamp),
    mPaymentOperationType(operationType),
    mContractorUUID(NodeUUID::empty()),
    mAmount(amount)
{}

const bool PaymentRecord::isPaymentRecord() const
{
    return true;
}

const PaymentRecord::PaymentOperationType PaymentRecord::paymentOperationType() const
{
    return mPaymentOperationType;
}

const NodeUUID PaymentRecord::contractorUUID() const
{
    return mContractorUUID;
}

const TrustLineAmount PaymentRecord::amount() const
{
    return mAmount;
}

const TrustLineBalance PaymentRecord::balanceAfterOperation() const
{
    return mBalanceAfterOperation;
}
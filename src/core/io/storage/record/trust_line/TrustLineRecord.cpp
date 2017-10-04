#include "TrustLineRecord.h"

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID):

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID,
        contractorUUID),
    mTrustLineOperationType(operationType),
    mAmount(0)
{}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount):

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID,
        contractorUUID),
    mTrustLineOperationType(operationType),
    mAmount(amount)
{}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const GEOEpochTimestamp geoEpochTimestamp) :

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID,
        contractorUUID,
        geoEpochTimestamp),
    mTrustLineOperationType(operationType),
    mAmount(amount)
{}

const bool TrustLineRecord::isTrustLineRecord() const
{
    return true;
}

const TrustLineRecord::TrustLineOperationType TrustLineRecord::trustLineOperationType() const
{
    return mTrustLineOperationType;
}

const TrustLineAmount TrustLineRecord::amount() const
{
    return mAmount;
}
#include "Record.h"

Record::Record(
    const Record::RecordType recordType,
    const TransactionUUID &operationUUID,
    Contractor::Shared contractor):

    mOperationUUID(operationUUID),
    mTimestamp(utc_now()),
    mContractor(contractor)
{
    mRecordType = recordType;
}

Record::Record(
    const Record::RecordType recordType,
    const TransactionUUID &operationUUID,
    Contractor::Shared contractor,
    const GEOEpochTimestamp geoEpochTimestamp) :

    mOperationUUID(operationUUID),
    mTimestamp(dateTimeFromGEOEpochTimestamp(geoEpochTimestamp)),
    mContractor(contractor)
{
    mRecordType = recordType;
}

const bool Record::isTrustLineRecord() const
{
    return false;
}

const bool Record::isPaymentRecord() const
{
    return false;
}

const Record::RecordType Record::recordType() const
{
    return mRecordType;
}

const TransactionUUID Record::operationUUID() const
{
    return mOperationUUID;
}

const DateTime Record::timestamp() const
{
    return mTimestamp;
}

Contractor::Shared Record::contractor() const
{
    return mContractor;
}
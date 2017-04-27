#include "Record.h"

Record::Record()
{}

Record::Record(
    const Record::RecordType recordType,
    const TransactionUUID &operationUUID):

    mOperationUUID(operationUUID),
    mTimestamp(utc_now())
{
    mRecordType = recordType;
}

Record::Record(
    const Record::RecordType recordType,
    const TransactionUUID &operationUUID,
    const GEOEpochTimestamp geoEpochTimestamp) :

    mOperationUUID(operationUUID)
{
        mRecordType = recordType;
        mTimestamp = dateTimeFromGEOEpochTimestamp(
            geoEpochTimestamp);
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
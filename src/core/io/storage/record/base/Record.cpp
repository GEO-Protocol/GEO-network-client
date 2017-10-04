#include "Record.h"

Record::Record()
{}

Record::Record(
    const Record::RecordType recordType,
    const TransactionUUID &operationUUID,
    const NodeUUID &contractorUUID):

    mOperationUUID(operationUUID),
    mTimestamp(utc_now()),
    mContractorUUID(contractorUUID)
{
    mRecordType = recordType;
}

Record::Record(
    const Record::RecordType recordType,
    const TransactionUUID &operationUUID,
    const NodeUUID &contractorUUID,
    const GEOEpochTimestamp geoEpochTimestamp) :

    mOperationUUID(operationUUID)
{
    mRecordType = recordType;
    mContractorUUID = contractorUUID,
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

const NodeUUID Record::contractorUUID() const
{
    return mContractorUUID;
}
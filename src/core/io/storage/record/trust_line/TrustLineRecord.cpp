#include "TrustLineRecord.h"

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    Contractor::Shared contractor):

    Record(
        Record::TrustLineRecordType,
        operationUUID,
        contractor),
    mTrustLineOperationType(operationType),
    mAmount(0)
{}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount):

    Record(
        Record::TrustLineRecordType,
        operationUUID,
        contractor),
    mTrustLineOperationType(operationType),
    mAmount(amount)
{}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const GEOEpochTimestamp geoEpochTimestamp) :

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID,
        contractor,
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

pair<BytesShared, size_t> TrustLineRecord::serializedHistoryRecordBody() const
{
    size_t recordBodySize = sizeof(SerializedTrustLineOperationType) + mContractor->serializedSize();
    if (mTrustLineOperationType != Closing &&
            mTrustLineOperationType != Rejecting) {
        recordBodySize += kTrustLineAmountBytesCount;
    }

    BytesShared bytesBuffer = tryCalloc(
        recordBodySize);
    size_t bytesBufferOffset = 0;

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &mTrustLineOperationType,
        sizeof(SerializedTrustLineOperationType));
    bytesBufferOffset += sizeof(SerializedTrustLineOperationType);

    auto contractorSerializedData = mContractor->serializeToBytes();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        contractorSerializedData.get(),
        mContractor->serializedSize());
    bytesBufferOffset += mContractor->serializedSize();

    if (mTrustLineOperationType != Closing &&
            mTrustLineOperationType != Rejecting) {
        auto trustAmountBytes = trustLineAmountToBytes(mAmount);

        memcpy(
            bytesBuffer.get() + bytesBufferOffset,
            trustAmountBytes.data(),
            kTrustLineAmountBytesCount);
    }
    return make_pair(
        bytesBuffer,
        recordBodySize);
}
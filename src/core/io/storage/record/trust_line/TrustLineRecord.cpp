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
    const GEOEpochTimestamp geoEpochTimestamp,
    BytesShared recordBody):
    Record(
        Record::TrustLineRecordType,
        operationUUID,
        geoEpochTimestamp)
{
    size_t dataBufferOffset = 0;
    auto* operationType =
        new (recordBody.get() + dataBufferOffset) TrustLineRecord::SerializedTrustLineOperationType;
    dataBufferOffset += sizeof(
        TrustLineRecord::SerializedTrustLineOperationType);
    mTrustLineOperationType = (TrustLineOperationType)*operationType;

    mContractor = make_shared<Contractor>(
        recordBody.get() + dataBufferOffset);
    dataBufferOffset += mContractor->serializedSize();

    mAmount = 0;
    if (*operationType != TrustLineRecord::TrustLineOperationType::Closing &&
        *operationType != TrustLineRecord::TrustLineOperationType::Rejecting) {
        vector<byte> amountBytes(
            recordBody.get() + dataBufferOffset,
            recordBody.get() + dataBufferOffset + kTrustLineAmountBytesCount);

        mAmount = bytesToTrustLineAmount(
            amountBytes);
    }
}

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
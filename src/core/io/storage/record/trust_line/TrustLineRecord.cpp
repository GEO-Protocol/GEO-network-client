#include "TrustLineRecord.h"

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    BytesShared buffer) :

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID)
{
    size_t dataBufferOffset = 0;

    SerializedTrustLineOperationType *operationType =
        new (buffer.get() + dataBufferOffset) SerializedTrustLineOperationType;
    mTrustLineOperationType = (TrustLineRecord::TrustLineOperationType) *operationType;
    dataBufferOffset += sizeof(
        SerializedTrustLineOperationType);

    memcpy(
        mContractorUUID.data,
        buffer.get() + dataBufferOffset,
        NodeUUID::kBytesSize);
    dataBufferOffset += NodeUUID::kBytesSize;

    if (mTrustLineOperationType != TrustLineRecord::TrustLineOperationType::Closing &&
        mTrustLineOperationType != TrustLineRecord::TrustLineOperationType::Rejecting) {
        vector<byte> amountBytes(
            buffer.get() + dataBufferOffset,
            buffer.get() + dataBufferOffset + kTrustLineAmountBytesCount);

        mAmount = bytesToTrustLineAmount(
            amountBytes);
    }
}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID):

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID),
    mTrustLineOperationType(operationType),
    mContractorUUID(contractorUUID),
    mAmount(0){}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount):

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID),
    mTrustLineOperationType(operationType),
    mContractorUUID(contractorUUID),
    mAmount(amount) {}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const GEOEpochTimestamp geoEpochTimestamp) :

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID,
        geoEpochTimestamp),
    mTrustLineOperationType(operationType),
    mContractorUUID(contractorUUID),
    mAmount(amount)
{}

const bool TrustLineRecord::isTrustLineRecord() const {

    return true;
}

const TrustLineRecord::TrustLineOperationType TrustLineRecord::trustLineOperationType() const{

    return mTrustLineOperationType;
}

const NodeUUID TrustLineRecord::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineAmount TrustLineRecord::amount() const {

    return mAmount;
}
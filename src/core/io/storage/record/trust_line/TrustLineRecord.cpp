#include "TrustLineRecord.h"

TrustLineRecord::TrustLineRecord(
    const uuids::uuid &operationUUID,
    BytesShared buffer) :

    Record(Record::RecordType::TrustLineRecordType,
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
    const uuids::uuid &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID):

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID),
    mTrustLineOperationType(operationType),
    mContractorUUID(contractorUUID) {}

TrustLineRecord::TrustLineRecord(
    const uuids::uuid &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount):

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID),
    mTrustLineOperationType(operationType),
    mContractorUUID(contractorUUID),
    mAmount(amount) {}

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

pair<BytesShared, size_t> TrustLineRecord::serializeToBytes() {

    size_t bytesCount = recordSize();

    BytesShared bytesBuffer = tryCalloc(
        bytesCount);
    size_t bytesBufferOffset = 0;

    SerializedTrustLineOperationType operationType = (SerializedTrustLineOperationType) mTrustLineOperationType;
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &operationType,
        sizeof(SerializedTrustLineOperationType));
    bytesBufferOffset += sizeof(
        SerializedTrustLineOperationType);

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        mContractorUUID.data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    if (mTrustLineOperationType != TrustLineRecord::TrustLineOperationType::Closing &&
        mTrustLineOperationType != TrustLineRecord::TrustLineOperationType::Rejecting) {
        auto trustAmountBytes = trustLineAmountToBytes(
            mAmount);

        memcpy(
            bytesBuffer.get() + bytesBufferOffset,
            trustAmountBytes.data(),
            kTrustLineAmountBytesCount);
    }

    return make_pair(
        bytesBuffer,
        bytesCount);
}

size_t TrustLineRecord::recordSize() {

    size_t result = sizeof(SerializedTrustLineOperationType) + NodeUUID::kBytesSize;
    if (mTrustLineOperationType != TrustLineRecord::TrustLineOperationType::Closing &&
        mTrustLineOperationType != TrustLineRecord::TrustLineOperationType::Rejecting) {
        result += kTrustLineAmountBytesCount;
    }
    return result;
}
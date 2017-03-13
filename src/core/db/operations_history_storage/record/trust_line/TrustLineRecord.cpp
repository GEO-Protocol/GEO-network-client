#include "TrustLineRecord.h"

namespace db {
    namespace operations_history_storage{

        TrustLineRecord::TrustLineRecord(
            BytesShared buffer) {

            deserializeFromBytes(
                buffer);
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

            auto parentBytesAndCount = Record::serializeToBytes();
            size_t bytesCount = kRecordBytesSize;

            BytesShared bytesBuffer = tryCalloc(
                kRecordBytesSize);
            size_t bytesBufferOffset = 0;

            memcpy(
                bytesBuffer.get(),
                parentBytesAndCount.first.get(),
                parentBytesAndCount.second);
            bytesBufferOffset += parentBytesAndCount.second;

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

            if (mTrustLineOperationType != TrustLineRecord::TrustLineOperationType::Closing) {
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

        void TrustLineRecord::deserializeFromBytes(
            BytesShared buffer) {

            Record::deserializeFromBytes(
                buffer);
            size_t dataBufferOffset = Record::kOffsetToInheritedBytes();

            SerializedTrustLineOperationType *operationType = new (buffer.get() + dataBufferOffset) SerializedTrustLineOperationType;
            mTrustLineOperationType = (TrustLineRecord::TrustLineOperationType) *operationType;
            dataBufferOffset += sizeof(
                SerializedTrustLineOperationType);

            memcpy(
                mContractorUUID.data,
                buffer.get() + dataBufferOffset,
                NodeUUID::kBytesSize);
            dataBufferOffset += NodeUUID::kBytesSize;

            if (mTrustLineOperationType != TrustLineRecord::TrustLineOperationType::Closing) {
                vector<byte> amountBytes(
                    buffer.get() + dataBufferOffset,
                    buffer.get() + dataBufferOffset + kTrustLineAmountBytesCount);

                mAmount = bytesToTrustLineAmount(
                    amountBytes);
            }
        }

    }
}
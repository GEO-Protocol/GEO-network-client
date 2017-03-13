#include "Record.h"

namespace db{
    namespace operations_history_storage{

        Record::Record() {}

        Record::Record(
            const Record::RecordType recordType,
            const uuids::uuid &operationUUID):

            mOperationUUID(operationUUID),
            mOperationTimestamp(utc_now()) {

            mRecordType = recordType;
        }

        const bool Record::isTrustLineRecord() const {

            return false;
        }

        const bool Record::isPaymentRecord() const {

            return false;
        }

        const Record::RecordType Record::recordType() const {

            return mRecordType;
        }

        const uuids::uuid Record::operationUUID() const {

            return mOperationUUID;
        }

        const DateTime Record::operationTimestamp() const {

            return mOperationTimestamp;
        }

        pair<BytesShared, size_t> Record::serializeToBytes() {

            size_t bytesCount = sizeof(SerializedRecordType)
                                + kOperationUUIDBytesSize
                                + sizeof(GEOEpochTimestamp);

            // tryCalloc using 'cause operation timestamp value may not fill int64_t type's range completely.
            BytesShared bytesBuffer = tryCalloc(
                bytesCount);
            size_t bytesBufferOffset = 0;

            SerializedRecordType recordType = (SerializedRecordType) mRecordType;
            memcpy(
                bytesBuffer.get(),
                &recordType,
                sizeof(SerializedRecordType));
            bytesBufferOffset += sizeof(
                SerializedRecordType);

            memcpy(
                bytesBuffer.get() + bytesBufferOffset,
                mOperationUUID.data,
                kOperationUUIDBytesSize);
            bytesBufferOffset += kOperationUUIDBytesSize;

            GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(mOperationTimestamp);
            memcpy(
                bytesBuffer.get() + bytesBufferOffset,
                &timestamp,
                sizeof(GEOEpochTimestamp));

            return make_pair(
                bytesBuffer,
                bytesCount);

        }

        void Record::deserializeFromBytes(
            BytesShared buffer) {

            size_t dataBufferOffset = 0;

            SerializedRecordType *recordType = new (buffer.get()) SerializedRecordType;
            mRecordType = (Record::RecordType) *recordType;
            dataBufferOffset += sizeof(
                SerializedRecordType);

            memcpy(
                mOperationUUID.data,
                buffer.get() + dataBufferOffset,
                kOperationUUIDBytesSize);
            dataBufferOffset += kOperationUUIDBytesSize;

            GEOEpochTimestamp *timestamp = new (buffer.get() + dataBufferOffset) GEOEpochTimestamp;
            mOperationTimestamp = dateTimeFromGEOEpochTimestamp(
                *timestamp);
        }

        const size_t Record::kOffsetToInheritedBytes() {

            static const size_t offset = sizeof(SerializedRecordType)
                                         + kOperationUUIDBytesSize
                                         + sizeof(GEOEpochTimestamp);
            return offset;
        }

    }
}

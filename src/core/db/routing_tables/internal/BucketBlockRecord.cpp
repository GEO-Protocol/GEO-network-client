#include "BucketBlockRecord.h"

namespace db {
namespace routing_tables {

BucketBlockRecord::BucketBlockRecord(const NodeUUID &uuid):
    mUUID(uuid),
    mRecordsNumbersCount(0),
    mRecordsNumbers(nullptr){}

BucketBlockRecord::BucketBlockRecord(const NodeUUID &uuid,
                                     RecordsCount recordsNumbersCount,
                                     RecordNumber *recordsNumbers):
    mUUID(uuid),
    mRecordsNumbers(recordsNumbers),
    mRecordsNumbersCount(recordsNumbersCount) {}

BucketBlockRecord::~BucketBlockRecord() {
    if (mRecordsNumbers != nullptr) {
        free(mRecordsNumbers);
    }
}

void BucketBlockRecord::insert(const RecordNumber recNo) {

    // Check if current record doesn't contains recNo.
    // If so - there is no reason to initialize buffers reorganization
    // and sorting operations.
    for (RecordsCount i=0; i<mRecordsNumbersCount; ++i){
        if (mRecordsNumbers[i] == recNo) {
            throw ConflictError(
                "BucketBlockRecord::insert: "
                    "duplicate recno occurred.");
        }
    }

    if (mRecordsNumbersCount == numeric_limits<RecordsCount>::max()){
        throw OverflowError(
            "BucketBlockRecord::insert: "
                "there is no free space in this record.");
    }


    const size_t newBufferSize =
        (mRecordsNumbersCount * sizeof(RecordNumber)) + sizeof(RecordNumber); // + one record

    RecordNumber *newBuffer = (RecordNumber*)malloc(newBufferSize);
    if (newBuffer == nullptr) {
        throw MemoryError(
            "BucketBlockRecord::insert: "
                "can't allocate memory for new records numbers block.");
    }

    // Copying values from previous buffer to the new one.
    if (mRecordsNumbersCount > 0) {
        memcpy(newBuffer, mRecordsNumbers, mRecordsNumbersCount*sizeof(RecordNumber));
        for (RecordsCount i=0; i<mRecordsNumbersCount; ++i){
            auto newBufferItem = newBuffer[i];
            if (recNo < newBufferItem) {
                new (newBuffer+i) RecordNumber(recNo);
                memcpy(newBuffer+i+1, mRecordsNumbers+i, (mRecordsNumbersCount-i)*sizeof(RecordNumber));

                goto EXIT;
            }
        }
    }

    newBuffer[mRecordsNumbersCount] = recNo;

EXIT:
    // Swap the buffers
    if (mRecordsNumbers != nullptr) {
        free(mRecordsNumbers);
    }
    mRecordsNumbers = newBuffer;
    mRecordsNumbersCount += 1;
}

bool BucketBlockRecord::remove(const RecordNumber recNo) {

    // Check if current record contains recNo.
    // If not - there is no reason to initialize buffers reorganization operations.
    for (RecordsCount i=0; i<mRecordsNumbersCount; ++i){
        if (mRecordsNumbers[i] == recNo) {
            const size_t newBufferSize = (mRecordsNumbersCount * sizeof(RecordNumber))
                                         - sizeof(RecordNumber); // - one record

            RecordNumber *newBuffer = (RecordNumber*)malloc(newBufferSize);
            if (newBuffer == nullptr) {
                throw MemoryError(
                    "BucketBlockRecord::remove: "
                        "can't allocate memory for new records numbers block.");
            }

            // Chain the buffers
            // (except removed element)
            if (i > 0) {
                memcpy(newBuffer, mRecordsNumbers, i*sizeof(RecordNumber));
            }
            memcpy(newBuffer+i, mRecordsNumbers+i+1, (mRecordsNumbersCount-i-1)*sizeof(RecordNumber));

            // Swap the buffers
            free(mRecordsNumbers);
            mRecordsNumbers = newBuffer;

            mRecordsNumbersCount -= 1;
            return true;
        }
    }

    return false;
}

const byte *BucketBlockRecord::data() const {
    return (byte*)mRecordsNumbers;
}

} // namespace routing_tables
} // namespace db

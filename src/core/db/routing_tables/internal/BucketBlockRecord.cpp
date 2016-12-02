#include "BucketBlockRecord.h"

namespace db {
namespace routing_tables {

BucketBlockRecord::BucketBlockRecord(const NodeUUID &uuid):
    mUUID(uuid),
    mRecordsNumbersCount(0),
    mRecordsNumbers(nullptr),
    mHasBeenModified(false){}

BucketBlockRecord::BucketBlockRecord(const NodeUUID &uuid,
                                     RecordsCount recordsNumbersCount,
                                     RecordNumber *recordsNumbers):
    mUUID(uuid),
    mRecordsNumbers(recordsNumbers),
    mRecordsNumbersCount(recordsNumbersCount),
    mHasBeenModified(false){}

BucketBlockRecord::~BucketBlockRecord() {
    if (mRecordsNumbers != nullptr) {
        free(mRecordsNumbers);
    }
}

/*!
 * Inserts "recNo" into the record.
 * In case when exact recNo is already present into the record -
 * throws "ConflictError".
 *
 * Throws OverflowError when no more rec. numbers may be inserted.
 * Throws MemoryError in case of lack of memory.
 */
void BucketBlockRecord::insert(const RecordNumber recN) {

    // Check if current record doesn't contains recN.
    // If so - there is no reason to initialize buffers reorganization
    // and sorting operations.
    for (RecordsCount i=0; i<mRecordsNumbersCount; ++i){
        if (mRecordsNumbers[i] == recN) {
            throw ConflictError(
                "BucketBlockRecord::insert: "
                    "duplicate recN occurred.");
        }
    }

    if (mRecordsNumbersCount == numeric_limits<RecordsCount>::max()){
        throw OverflowError(
            "BucketBlockRecord::insert: "
                "there is no free space in this record.");
    }


#define REC_N_SIZE sizeof(RecordNumber)

    const size_t newBufferSize =
        (mRecordsNumbersCount * REC_N_SIZE) + REC_N_SIZE; // + one record

    RecordNumber *newBuffer = (RecordNumber*)malloc(newBufferSize);
    if (newBuffer == nullptr) {
        throw MemoryError(
            "BucketBlockRecord::insert: "
                "can't allocate memory for new records numbers block.");
    }

    // Copying values from previous buffer to the new one.
    if (mRecordsNumbersCount > 0) {
        memcpy(newBuffer, mRecordsNumbers, mRecordsNumbersCount*REC_N_SIZE);
        for (RecordsCount i=0; i<mRecordsNumbersCount; ++i){
            auto newBufferItem = newBuffer[i];
            if (recN < newBufferItem) {
                new (newBuffer+i) RecordNumber(recN);
                memcpy(newBuffer+i+1, mRecordsNumbers+i, (mRecordsNumbersCount-i)*REC_N_SIZE);

                goto EXIT;
            }
        }
    }

    newBuffer[mRecordsNumbersCount] = recN;

EXIT:
    // Swap the buffers
    if (mRecordsNumbers != nullptr) {
        free(mRecordsNumbers);
    }
    mRecordsNumbers = newBuffer;
    mRecordsNumbersCount += 1;
    mHasBeenModified = true;
}

/*!
 * Removes "recNo" from the record.
 * Returns true in case of success, otherwise - returns false.
 *
 * Throws MemoryError in case of lack of memory.
 */
bool BucketBlockRecord::remove(const RecordNumber recN) {

    // Check if current record contains recN.
    // If not - there is no reason to initialize buffers reorganization operations.
    for (RecordsCount i=0; i<mRecordsNumbersCount; ++i){
        if (mRecordsNumbers[i] == recN) {
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
            mHasBeenModified = true;
            return true;
        }
    }

    return false;
}

const byte *BucketBlockRecord::data() const {
    return (byte*)mRecordsNumbers;
}

const NodeUUID &BucketBlockRecord::uuid() const {
    return mUUID;
}

const bool BucketBlockRecord::isModified() const {
    return mHasBeenModified;
}

/*!
 * Returns index of "recN" in the record.
 * In case if such "recN" is absent - throws IndexError.
 */
const RecordsCount BucketBlockRecord::indexOf(const RecordNumber recN) {
    RecordsCount first = 0;
    RecordsCount last = mRecordsNumbersCount; // index of elem. that is BEHIND THE LAST elem.

    if (mRecordsNumbersCount == 0) {
        // The record is empty.
        // There is no reason to search anything;
        throw IndexError(
            "BucketBlockRecord::indexOf: "
                "record is empty. No search operation is possible.");

    } else if (recN < mRecordsNumbers[0]) {
        // Record record number is less than least one.
        throw IndexError(
            "BucketBlockRecord::indexOf: "
                "recN is absent in the row.");

    } else if (recN > mRecordsNumbers[mRecordsNumbersCount - 1]) {
        // Received record number is greater than the greatest one.
        throw IndexError(
            "BucketBlockRecord::indexOf: "
                "recN is absent in the row.");
    }

    while (first < last) {
        size_t mid = first + (last - first) / 2;

        if (recN <= mRecordsNumbers[mid])
            last = mid;
        else
            first = mid + 1;
    }

    if (mRecordsNumbers[last] == recN) {
        // Found.
        return last;

    } else {
        throw IndexError(
            "BucketBlockRecord::indexOf: "
                "recN is absent in the row.");
    }
}

} // namespace routing_tables
} // namespace db

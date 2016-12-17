#include "BucketBlock.h"


namespace db {
namespace fields {
namespace uuid_map {

BucketBlock::BucketBlock():
    mRecords(nullptr),
    mRecordsCount(0),
    mHasBeenModified(false){}

BucketBlock::BucketBlock(BucketBlockRecord *records, const RecordNumber recordsCount):
    mRecords(records),
    mRecordsCount(recordsCount),
    mHasBeenModified(false){}

BucketBlock::~BucketBlock() {
    for (size_t i=0; i<mRecordsCount; ++i){
        delete (mRecords+i);
    }
}

/*!
 * Inserts record with "uuid" and "recN" into the block.
 * In case when record with "uuid" would be already present -
 * "recN" would be inserted into it with reallocation.
 * Otherwise - new record would be created.
 *
 * It is guarantied, that "mRecords" will remain sorted in ascending order.
 *
 * Throws "ConflictError" in case when record with "uuid" already contains exact "recN".
 * Throws "MemoryError".
 */
void BucketBlock::insert(const NodeUUID &uuid, const RecordNumber recN) {
    if (mRecordsCount == numeric_limits<AbstractRecordsHandler::RecordsCount>::max()){
        throw OverflowError(
            "BucketBlock::set: "
                "there is no free space in this block.");
    }

    auto record = recordByUUID(uuid);
    if (record == nullptr) {
        // Current block doesn't contains record with exact uuid.
        // New one record should be created.
        record = createRecord(uuid);
    }

    record->insert(recN);
    mHasBeenModified = true;
}

/*!
 * Returns true in case when record with "uuid" and "recN" was successfully removed.
 * Otherwise - returns false.
 */
bool BucketBlock::remove(const NodeUUID &uuid, const RecordNumber recN) {
    auto record = recordByUUID(uuid);
    if (record == nullptr) {
        // There is no record with exact uuid in the block.
        return false;
    }

    bool isRecordWasRemoved = record->remove(recN);
    if (isRecordWasRemoved) {
        mHasBeenModified = true;

        if (record->count() == 0)
            dropRecord(record);
    }
    return isRecordWasRemoved;
}

/*!
 * Returns nullptr in case when record with "uuid" was not found in the block;
 * Otherwise - returns it's address.
 */
BucketBlockRecord *BucketBlock::recordByUUID(const NodeUUID &uuid) const {
    try {
        AbstractRecordsHandler::RecordsCount index = recordIndexByUUID(uuid);
        return mRecords+index;

    } catch (IndexError &) {
        return nullptr;
    }
}

/*!
 * Uses binary search.
 * Returns record index in case when record with "uuid" was found in the block;
 * Otherwise - throws IndexError;
 */
const AbstractRecordsHandler::RecordsCount BucketBlock::recordIndexByUUID(const NodeUUID &uuid) const {
    AbstractRecordsHandler::RecordsCount first = 0;
    AbstractRecordsHandler::RecordsCount last = mRecordsCount; // index of elem. that is BEHIND THE LAST elem.

    if (mRecordsCount == 0) {
        // The record is empty.
        // There is no reason to search anything;
        throw IndexError(
            "BucketBlockRecord::indexOf: "
                "record is empty. No search operation is possible.");

    } else if (NodeUUID::compare(uuid, mRecords[0].uuid()) == NodeUUID::LESS) {
        // Record record number is less than least one.
        throw IndexError(
            "BucketBlockRecord::indexOf: "
                "recN is absent in the row.");

    } else if (NodeUUID::compare(uuid, mRecords[mRecordsCount-1].uuid()) == NodeUUID::GREATER) {
        // Received record number is greater than the greatest one.
        throw IndexError(
            "BucketBlockRecord::indexOf: "
                "recN is absent in the row.");
    }

    while (first < last) {
        size_t mid = first + (last - first) / 2;

        auto comp = NodeUUID::compare(uuid, mRecords[mid].uuid());
        if (comp == NodeUUID::LESS || comp == NodeUUID::EQUAL)
            last = mid;
        else
            first = mid + 1;
    }

    if (NodeUUID::compare(uuid, mRecords[last].uuid()) == NodeUUID::EQUAL) {
        // Found.
        return last;

    } else {
        throw IndexError(
            "BucketBlockRecord::indexOf: "
                "recN is absent in the row.");
    }
}

const bool BucketBlock::isModified() const {
    return mHasBeenModified;
}

BucketBlockRecord *BucketBlock::createRecord(const NodeUUID &uuid) {
    BucketBlockRecord *newRecordAddress = nullptr;

#define BUCKET_BLOCK_RECORD_PTR_SIZE sizeof(BucketBlockRecord)

    // Put newly create record into the right position in the records buffer.
    // Records must stay in sorted order (ascending).
    const size_t newBufferSize = (mRecordsCount * BUCKET_BLOCK_RECORD_PTR_SIZE)
        + BUCKET_BLOCK_RECORD_PTR_SIZE; // + one record

    BucketBlockRecord *newBuffer = (BucketBlockRecord*)malloc(newBufferSize);
    if (newBuffer == nullptr) {
        throw MemoryError(
            "BucketBlock::createRecord: "
                "can't allocate memory for new recordNumbers buffer.");
    }

    // Copying values from previous buffer to the new one.
    if (mRecordsCount > 0) {
        memcpy(newBuffer, mRecords, mRecordsCount*BUCKET_BLOCK_RECORD_PTR_SIZE);
        for (AbstractRecordsHandler::RecordsCount i=0; i<mRecordsCount; ++i){
            BucketBlockRecord *bufferRecord = newBuffer+i;
            auto compareResult = NodeUUID::compare(uuid, bufferRecord->uuid());
            if (compareResult == NodeUUID::LESS) {
                newRecordAddress = newBuffer+i;
                new (newRecordAddress) BucketBlockRecord(uuid);
                memcpy(newBuffer+i+1, mRecords+i, (mRecordsCount-i)*BUCKET_BLOCK_RECORD_PTR_SIZE);

                goto EXIT;
            }
        }
    }

    newRecordAddress = newBuffer+mRecordsCount;
    new (newRecordAddress) BucketBlockRecord(uuid);

EXIT:
    // Swap the buffers
    if (mRecords != nullptr) {
        free(mRecords);
    }
    mRecords = newBuffer;
    mRecordsCount += 1;

    return newRecordAddress;
}

void BucketBlock::dropRecord(BucketBlockRecord *record) {
    try {
        AbstractRecordsHandler::RecordsCount index = recordIndexByUUID(record->uuid());

        const size_t newBufferSize = (mRecordsCount * BUCKET_BLOCK_RECORD_PTR_SIZE)
            - sizeof(BucketBlockRecord); // - one record

        BucketBlockRecord *newBuffer = (BucketBlockRecord*)malloc(newBufferSize);
        if (newBuffer == nullptr) {
            throw MemoryError(
                "BucketBlockRecord::remove: "
                    "can't allocate memory for new recordNumbers numbers block.");
        }

        // Chain the buffers
        // (except removed element)
        if (index > 0) {
            memcpy(newBuffer, mRecords, index*BUCKET_BLOCK_RECORD_PTR_SIZE);
        }
        memcpy(newBuffer+index, mRecords+index+1,
               (mRecordsCount-index-1)*BUCKET_BLOCK_RECORD_PTR_SIZE);

        // Swap the buffers
        free(mRecords);
        mRecords = newBuffer;

        mRecordsCount -= 1;
        mHasBeenModified = true;

    } catch (IndexError &){
        return;
    }
}

const AbstractRecordsHandler::RecordNumber BucketBlock::recordsCount() const {
    return mRecordsCount;
}

const BucketBlockRecord *BucketBlock::records() const {
    return mRecords;
}

/*
 * Returns pointer to block data and it's size in bytes.
 */
const pair<void*, size_t> BucketBlock::data() const {
    return make_pair((void*)mRecords, mRecordsCount * sizeof(RecordNumber));
}


} // namespace uuid_map
} // namespace fields
} // namespace db




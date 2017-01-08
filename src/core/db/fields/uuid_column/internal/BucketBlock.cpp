#include "BucketBlock.h"


namespace db {
namespace fields {
namespace uuid_map {


BucketBlock::BucketBlock():
    mRecords(nullptr),
    mRecordsCount(0),
    mHasBeenModified(false){}

/*!
 * Deserialization constructor.
 *
 * Initializes the instance from the already allocated and populated data-block.
 * This constructor is used to perform optimized instance initialisation after reading from the disk.
 *
 * @param data - pointer to the data block, that was read from the disk.
 */
BucketBlock::BucketBlock(
    byte *data):
    mHasBeenModified(false){

    // By the format, each serialized bucket block is prefixed with records count field.
    const auto kRecordsCount = *((RecordsCount*)data);
    mRecords = (BucketBlockRecord*)malloc(sizeof(BucketBlockRecord) * kRecordsCount);
    if (mRecords == nullptr) {
        throw MemoryError(
            "BucketBlock::BucketBlock: bad alloc.");
    }
    mRecordsCount = kRecordsCount;

    // Populating the records of the block with the data.
    BucketBlockRecord* nextRecordOffset = mRecords;
    byte *currentDataOffset = data + sizeof(RecordsCount);

    for (RecordsCount i=0; i<kRecordsCount; ++i) {
        // Create instance of BucketBlockRecord into
        // the already allocated and populated data-block;
        new (nextRecordOffset) BucketBlockRecord(currentDataOffset);

        // Determine how much record numbers are present into the BucketBlockRecord.
        // This is needed to be able to increment offset for the next record.
        // (Records numbers count field is situated after the uuid field)
        byte *recordsCountFieldOffset = currentDataOffset + NodeUUID::kBytesSize;
        RecordsCount recordNumbersCount = *(RecordsCount*)recordsCountFieldOffset;

        // Move offset to the next record
        currentDataOffset +=
            + NodeUUID::kBytesSize
            + sizeof(RecordsCount)
            + sizeof(RecordNumber) * recordNumbersCount;

        nextRecordOffset += 1;
    }
}

BucketBlock::~BucketBlock() {
    if (mRecords != nullptr) {
        for (size_t i=0; i<mRecordsCount; ++i){
            delete (mRecords+i);
        }

        mRecords = nullptr;
    }
}

/*!
 * Inserts record with "uuid" and "recN" into the block.
 * In case when record with "uuid" is already present -
 * "recN" would be appended to the record, associated with "uuid" (with reallocation).
 * Otherwise - new record would be created.
 *
 * It is guarantied, that "mRecords" will remain sorted in ascending order.
 *
 *
 * Throws OverflowError in case when there is no free space into the block.
 * Throws ConflictError in case when record with "uuid" already contains exact "recN".
 * Throws MemoryError.
 */
void BucketBlock::insert(const NodeUUID &uuid, const RecordNumber recN) {
    if (mRecordsCount == numeric_limits<AbstractRecordsHandler::RecordsCount>::max()){
        throw OverflowError(
            "BucketBlock::set: "
                "there is no free space in this block.");
    }

    try {
        auto record = recordByUUID(uuid);
        record->insert(recN);

    } catch (IndexError &) {
        // Current block doesn't contains record with exact uuid.
        // New one record should be created.
        auto record = createRecord(uuid);
        record->insert(recN);
    }
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

    bool isRecNWasRemoved = record->remove(recN);
    if (isRecNWasRemoved) {
        mHasBeenModified = true;

        if (record->count() == 0)
            dropRecord(record);
    }
    return isRecNWasRemoved;
}

/*!
 * Returns pointer to the record by it's uuid.
 *
 *
 * Throws IOError in case when no record with exact uuid is present in the block;
 */
BucketBlockRecord *BucketBlock::recordByUUID(
    const NodeUUID &uuid) const {

    auto index = recordIndexByUUID(uuid);
    return mRecords+index;
}

/*!
 * Uses binary search.
 * Returns record index in case when record with "uuid" was found in the block;
 * Otherwise - throws IndexError;
 */
const AbstractRecordsHandler::RecordsCount BucketBlock::recordIndexByUUID(
    const NodeUUID &uuid) const {

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
        RecordsCount mid = first + (last - first) / 2;

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

const BucketBlockRecord* BucketBlock::records() const {
    return mRecords;
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


const pair<shared_ptr<byte>, uint32_t> BucketBlock::serializeToBytes() const {

    uint32_t totalBlockSize = 0;

    // Block data should be prefixed with field,
    // that specifies how many records are in the block.
    totalBlockSize += sizeof(RecordNumber);

    // Calculating how long the block is.
    for (RecordsCount i=0; i<mRecordsCount; ++i){
        totalBlockSize +=
            + NodeUUID::kBytesSize  // uuid of the record
            + sizeof(RecordsCount)  // how many record numbers are stored in the record
            + sizeof(RecordNumber) * mRecords[i].count(); // record numbers itself
    }

    byte *block = (byte*) malloc(totalBlockSize);
    if (block == nullptr) {
        throw MemoryError(
            "BucketBlock::serializeToBytes: "
                "can't allocated memory for serialization.");
    }

    //
    // Filling the block with data
    //

    // Filling records count
    new (block) RecordsCount(mRecordsCount);

    // Filling records
    byte *currentOffset = block + sizeof(RecordNumber);
    for (RecordsCount i=0; i<mRecordsCount; ++i){
        // UUID
        memcpy(currentOffset, mRecords[i].uuid().data, NodeUUID::kBytesSize);
        currentOffset += NodeUUID::kBytesSize;

        // Record numbers count
        new (currentOffset) RecordsCount(mRecords[i].count());
        currentOffset += sizeof(RecordsCount);

        // Record numbers
        memcpy(currentOffset,  mRecords[i].recordNumbers(), sizeof(RecordNumber) * mRecords[i].count());
        currentOffset += sizeof(RecordNumber) * mRecords[i].count();
    }

    shared_ptr<byte> blockPtr(block, free);
    return make_pair(blockPtr, totalBlockSize);
}


} // namespace uuid_map
} // namespace fields
} // namespace db




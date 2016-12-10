#include "UUIDMapStorage.h"


namespace db {
namespace fields {
namespace uuid_map {

UUIDMapStorage::UUIDMapStorage(const char *filename,
                 const char *path,
                 const uint8_t pow2BucketsCountIndex):
    AbstractFileDescriptorHandler(filename, path),
    mPow2BucketsCountIndex(pow2BucketsCountIndex){

    open(kWriteAccessMode);
}

/*!
 * Inserts "uuid" into the map.
 *
 * Throws "ConflictError" in case, when equal record is already present in the block.
 * Throws "OverflowError" in case when block is full and no more records may be inserted.
 * Throws "MemoryError" in case when there is not enough memory.
 */
void UUIDMapStorage::set(const NodeUUID &uuid,
                         const BucketRecordNumber recN) {

    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto bucket = getBucket(bucketIndex);

    bucket->insert(uuid, recN);
    cacheBucketBlock(bucketIndex, bucket);
}

/*!
 * @param uuid - uuid of the record, that should be removed.
 * @param recN - record number of the record,  that should be removed.
 *
 * @return true if record with "uuid" and "recN" was removed,
 * otherwise returns false.
 *
 * Note: one UUID may be linked with several record numbers,
 * so removing of the record only via uuid.
 */
const bool UUIDMapStorage::remove(const NodeUUID &uuid,
                           const UUIDMapStorage::BucketRecordNumber recN) {

    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto block = getBucket(bucketIndex);

    bool removed = block->remove(uuid, recN);
    if (removed) {
        cacheBucketBlock(bucketIndex, block);
    }

    return removed;
}

const UUIDMapStorage::BucketRecordNumber UUIDMapStorage::bucketRecordIndexByUUID(const NodeUUID &uuid) const {
    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto bucket = getBucket(bucketIndex);

    return bucket->recordIndexByUUID(uuid);
}

/*!
 * Writes empty header to the file.
 * See the BucketBlockDescriptor class docs for the details.
 */
void UUIDMapStorage::initFileHeader() {
    // Initialising memory buffer for empty header record.
    byte *emptyBucketsHeaderRecord = (byte *)malloc(kBucketsHeaderRecordSize);
    if (emptyBucketsHeaderRecord == nullptr) {
        throw MemoryError(
            "UUIDMapStorage::initFileHeader "
                "can't allocate enough memory for empty buckets header record.");
    }
    memset(emptyBucketsHeaderRecord, 0, kBucketsHeaderRecordSize);

    // Writing empty record header for each one bucket.
    fseek(mFileDescriptor, 0, SEEK_SET);

    const auto bucketsCount = totalBucketsCount();
    for (size_t i = 0; i < bucketsCount; ++i) {
        auto recordsWritten = fwrite(
            emptyBucketsHeaderRecord, kBucketsHeaderRecordSize, 1, mFileDescriptor);

        if (recordsWritten != 1) {
            free(emptyBucketsHeaderRecord);

            throw IOError(
                "UUIDMapStorage::initFileHeader: "
                    "can't write empty bucket header to the disk.");
        }
    }

    // Seems to be OK
    free(emptyBucketsHeaderRecord);
}

void UUIDMapStorage::open(const char *accessMode) {
    AbstractFileDescriptorHandler::open(accessMode);

    if (fileSize() == 0) {
        initFileHeader();
    }
}

const size_t UUIDMapStorage::totalBucketsCount() const {
    return pow(2, mPow2BucketsCountIndex);
}

/*!
 * @param u - uuid for which the bucket index should be calculated.
 * @return index (not address) of the bucket, that contains the "u".
 * To get the bucket - this index should be multiplied by the record size.
 */
const UUIDMapStorage::BucketIndex UUIDMapStorage::bucketIndexByNodeUUID(const NodeUUID &u) const {
    // todo: ensure buckets sorting in ascending order

    // Bucket index is the mask,
    // formed by chaining significant bits from several bytes of the "nodeUUID".
    // "mPow2BucketsCountIndex" determines how many buckets may be generated,
    // and how many bits should be used in the mask to handle their indexes.
    //
    // In case when "mPow2BucketsCountIndex" is smaller than length of the uuid (16B),
    // significant bits should be collected from the uuid in way of normal distribution.
    // "middleOffset" makes it possible to use several bytes from the middle of the uuid,
    // and guarantee in such a way that significant bits would be collected
    // in random order.
    //
    // Note:
    // this approach assumes good normal distribution of the random generator,
    // that was used for generating of the uuid.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
    const byte middleOffset = (kUUIDLength - mPow2BucketsCountIndex) / 2;
#pragma clang diagnostic pop

    BucketIndex index = 0;
    for (byte i=0; i<mPow2BucketsCountIndex; ++i){
        index = (index << 8) | u.data[i+middleOffset];
    }
    return index;
}

BucketBlock* UUIDMapStorage::getBucket(const UUIDMapStorage::BucketIndex index) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(index < totalBucketsCount());
#endif

    if (mBlocksCache.count(index) > 0) {
        return mBlocksCache.at(index);

    } else {
        // Cache doesn't contains the block.

        try {
            auto bucketBlock = readBucket(index);
            cacheBucketBlock(index, bucketBlock);
            return bucketBlock;

        } catch (NoBucketBlockError) {
            // It seems, that file doesn't contains block for such bucket.
            // New one should be created, cached and then - returned.
            //
            // It is OK to cache empty block.
            // in case if it would not be modified -
            // it would not be written to the disk

            try {
                auto bucketBlock = new BucketBlock();
                cacheBucketBlock(index, bucketBlock);
                return bucketBlock;

            } catch (...) {
                throw MemoryError(
                    "UUIDMapStorage::getBucket: "
                        "not enough memory for creating empty bucket block.");
            }
        }
    }
}

/*!
 * Tries to read bucket block from the disk.
 * Returns it in case of success;
 *
 * Throws "NoBucketBlockError" in case if block doesn't exists in file.
 * Throws "MemoryError".
 *
 * @param index - index of bucket of which block should be returned.
 */
BucketBlock* UUIDMapStorage::readBucket(BucketIndex index) const {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(index < totalBucketsCount());
#endif

    struct {
        BucketBlockAddress address;
        BucketRecordsCount recordsCount;
    } header;

    fseek(mFileDescriptor, index * kBucketsHeaderRecordSize, SEEK_SET);
    fread(&header, kBucketsHeaderRecordSize, 1, mFileDescriptor);
    if (header.address == kNoBucketAddressValue) {
        // There is no block assigned to this bucket.
        throw NoBucketBlockError(
            "UUIDMapStorage::readBucket: "
                "there is no bucket block associated with this bucket.");
    }
    return readBucketBlock(header.address, header.recordsCount);
}

/*!
 * Tries to read bucket block from the disk.
 * Returns it in case of success;
 *
 * Throws "MemoryError"
 *
 * @param address - specifies address of the block in the file.
 * @param recordsCount - specifies how many records should be read starting from "address".
 */
BucketBlock* UUIDMapStorage::readBucketBlock(const UUIDMapStorage::BucketBlockAddress address,
                                             const UUIDMapStorage::BucketRecordsCount recordsCount) const {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(address >= kBucketsHeaderRecordSize * totalBucketsCount());
    assert(recordsCount > 0);
#endif

    byte *blockData = (byte*) malloc(recordsCount * kBucketsHeaderRecordSize);
    if (blockData == nullptr) {
        throw MemoryError(
            "UUIDMapStorage::readBucketBlock: "
                "can't allocate memory for the bucket block.");
    }

    fseek(mFileDescriptor, address, SEEK_SET);
    auto recordsRead = fread(
        blockData, recordsCount * kBucketsHeaderRecordSize, 1, mFileDescriptor);

    if (recordsRead != 1) {
        free(blockData);
        throw MemoryError(
            "UUIDMapStorage::readBucketBlock: "
                "can't dataOffset block from the disk.");
    }

    // Note:
    // BucketBlock will take ownership on "blockData".
    return new BucketBlock((BucketBlockRecord*)blockData, recordsCount);
}

/*!
 * Caches the "block" by the bucket index "index";
 * Updates the cache in case when "block" is already present in the block.
 */
void UUIDMapStorage::cacheBucketBlock(const BucketIndex index, BucketBlock *block) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(index < totalBucketsCount());
    assert(block != nullptr);
#endif

    mBlocksCache.insert(make_pair(index, block));
}

const pair<AbstractRecordsHandler::RecordNumber *, AbstractRecordsHandler::RecordsCount>
UUIDMapStorage::recordNumbersAssignedToUUID(const NodeUUID &uuid) {

    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto bucket = getBucket(bucketIndex);

    try {
        auto record = bucket->recordByUUID(uuid);
        return make_pair(record->records(), record->count());

    } catch (IndexError &e) {
        // There is no such uuid in the bucket block.
        // As a result - there is no records assigned to this uuid.
        return make_pair(nullptr, 0);
    }
}


}
}
}

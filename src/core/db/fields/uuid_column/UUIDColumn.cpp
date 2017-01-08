#include "UUIDColumn.h"


namespace db {
namespace fields {

/*!
 * @param path - path to te directory in which column should be created.
 * @param pow2BucketsCountIndex - specifies how many buckets would be created in the column.
 * This argument is represented as power of 2, for example:
 * if pow2BucketsCountIndex would be equal to 2 - than 4 buckets would be created,
 * if pow2BucketsCountIndex would be equal to 10 - than 2**10 buckets would be created.
 * See class description for more details about buckets.
 *
 *
 * Throws ValueError in case if one or more arguments doens't pass validation.
 * Throws RuntimeError in case if internal data file can't be opened.
 */
UUIDColumn::UUIDColumn(
    const fs::path &path,
    const uint8_t pow2BucketsCountIndex):

    AbstractFileDescriptorHandler(path / fs::path(kDataFilename)),
    mRecordsIndex(path / fs::path(kIndexFilename)),
    mPow2BucketsCountIndex(pow2BucketsCountIndex){

    // todo: in case when data file is already present - mPow2BucketsCountIndex and the rest data must be readed from the disk.

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(! path.has_extension());     // path should not indicate the file.
#endif

    if (pow2BucketsCountIndex == 0 || pow2BucketsCountIndex > 16) {
        throw ValueError(
            "UUIDColumn::UUIDColumn: "
                "\"pow2BucketsCountIndex\" can't be equal to 0 or greater than NodeUUID::kBytesSize.");
    }

    try {
        open();

    } catch (Exception &e) {
        throw RuntimeError(
            string("UUIDColumn::UUIDColumn: can't open file descriptor. Details: ")
            + e.message());
    }
}


UUIDColumn::FileHeader::FileHeader():
    version(1),
    state(READ_WRITE) {}

UUIDColumn::FileHeader::FileHeader(
    const uint16_t version,
    const uint8_t state):

    version(version),
    state(state) {}


/*!
 * Tries to assign "recN" to the "uuid".
 * If "sync" is "true" - internal read/write cache would be synced to the disk.
 *
 *
 * Throws ConflictError in case, when equal record is already present in the block.
 * Throws OverflowError in case when block is full and no more records may be inserted.
 * Throws MemoryError;
 * Throws IOError;
 */
void UUIDColumn::set(
    const NodeUUID &uuid,
    const BucketRecordNumber recN,
    const bool sync) {

    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto bucketBlock = getBucketBlock(bucketIndex);

    bucketBlock->insert(uuid, recN);
    cacheBucketBlock(bucketIndex, bucketBlock);

    if (sync) {
        commitCachedBlocks();
    }
}

/*!
 * Attempts to remove record (uuid->recN) from the storage.
 * Returns "true" if record was removed successfully,
 * otherwise returns "false".
 *
 * Note: several record numbers may be assigned to one UUID.
 * So the UUID would be removed fro mthe storage only in case
 * if last recN would deattached from this UUID.
 */
const bool UUIDColumn::remove(
    const NodeUUID &uuid,
    const UUIDColumn::BucketRecordNumber recN,
    const bool commit) {

    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto bucketBlock = getBucketBlock(bucketIndex);

    bool succesfullyRemoved = bucketBlock->remove(uuid, recN);
    if (succesfullyRemoved) {
        cacheBucketBlock(bucketIndex, bucketBlock);

        if (commit) {
            commitCachedBlocks();
        }
    }
    return succesfullyRemoved;
}

/*!
 * Opens internal data file for writing.
 * In case if it was created a moment ago - also writes initial file header to it.
 */
void UUIDColumn::open() {
    AbstractFileDescriptorHandler::open();

    if (fileSize() == 0) {
        initDefaultFileHeader();
        initBucketsIndex();

    } else {
        initFromFileHeader();
    }
}

const size_t UUIDColumn::totalBucketsCount() const {
    return (size_t)pow(double(2), mPow2BucketsCountIndex);
}

/*!
 * @param u - uuid for which the bucket index should be calculated.
 * @return index (not address) of the bucket, that contains the "u".
 * To get the bucket - this index should be multiplied by the record size.
 */
const UUIDColumn::BucketIndex UUIDColumn::bucketIndexByNodeUUID(
    const NodeUUID &u) const {

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

    uint16_t index = 0;
    for (uint8_t i=0; i<mPow2BucketsCountIndex; ++i) {
        const uint8_t byte = u.data[i];

        if (byte & 1) {
            index |= 1;
        } else {
            index |= 0;
        }
        index = index << 1;
    }

    return index >> 1; // last transaction shouldn't use shift.
}

/*!
 * Tries to return block from the cache.
 * If cache doesn't contains the block - tries to load it from the disk.
 * In case if block is not listed on the disk - it will be created.
 *
 * @param index - specifies what bucket block should be returned;
 */
SharedBucketBlock UUIDColumn::getBucketBlock(
    const UUIDColumn::BucketIndex index) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(index < totalBucketsCount());
#endif

    if (mReadWriteCache.count(index) > 0) {
        return mReadWriteCache.at(index);

    } else {
        // Cache doesn't contains the block.

        try {
            auto block = readBucket(index);
            cacheBucketBlock(index, block);
            return block;

        } catch (NotFoundError &) {
            // It seems, that file doesn't contains block for such bucket.
            // New one should be created, cached and then - returned.
            //
            // It is OK to cache empty block.
            // in case if it would not be modified -
            // it would not be written to the disk

            try {
                auto block = SharedBucketBlock(new BucketBlock());
                cacheBucketBlock(index, block);
                return block;

            } catch (std::bad_alloc &) {
                throw MemoryError(
                    "UUIDColumn::getBucketBlock: "
                        "not enough memory for creating empty bucket block.");
            }
        }
    }
}

/*!
 * Tries to read buckets index and locate the bucket block.
 * In case of success - calls the readBucketBlock() method and returns it's result.
 *
 * Throws "NotFoundError" if index doesn't contains offset of the bucket.
 * Throws "IOError" if read operation failed.
 * Throws "MemoryError" if memory operation failed.
 *
 * @param index - index of bucket that should be returned.
 */
SharedBucketBlock UUIDColumn::readBucket(
    const BucketIndex index) const {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(index < totalBucketsCount());
#endif

    seekToBucketIndexRecord(index);

    BucketsIndexRecord indexRecord;
    if (fread(&indexRecord, sizeof(BucketsIndexRecord), 1, mFileDescriptor) != 1 &&
        fread(&indexRecord, sizeof(BucketsIndexRecord), 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDColumn::readBucket: "
                "can't read buckets index record.");
    }

    if (indexRecord.offset == kNoBucketAddressValue) {
        throw NotFoundError(
            "UUIDColumn::readBucket: "
                "there is no bucket block associated with this bucket.");
    }

    return readBlock(indexRecord);
}

/*
 * Seeks the file descriptor to the buckets index record with position "index".
 */
void UUIDColumn::seekToBucketIndexRecord(
    UUIDColumn::BucketIndex index) const {

    const auto offset =
        + sizeof(FileHeader)
        + sizeof(BucketsIndexRecord) * index;

    fseek(mFileDescriptor, offset, SEEK_SET);
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
SharedBucketBlock UUIDColumn::readBlock(
    const BucketsIndexRecord indexRecord) const {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(indexRecord.offset >= sizeof(FileHeader));
    assert(indexRecord.bytesCount > 0);
#endif

    if (fseek(mFileDescriptor, indexRecord.offset, SEEK_SET) != 0){
        throw IOError(
            "UUIDColumn::readBlock: "
                "can't seek to the bucket block.");
    }

    byte *blockData = (byte*) malloc(indexRecord.bytesCount);
    if (blockData == nullptr) {
        throw MemoryError(
            "UUIDColumn::readBlock: "
                "can't allocate memory for the bucket block.");
    }

    if (fread(blockData, indexRecord.bytesCount, 1, mFileDescriptor) != 1 &&
        fread(blockData, indexRecord.bytesCount, 1, mFileDescriptor) != 1) {
        free(blockData);
        throw MemoryError(
            "UUIDColumn::readBlock: "
                "can't dataOffset block from the disk.");
    }

    try {
        SharedBucketBlock block(new BucketBlock(blockData));
        free(blockData);
        return block;

    } catch (bad_alloc &) {
        throw MemoryError(
            "UUIDColumn::readBlock: bad alloc.");
    }

}

/*!
 * Caches the "block" by the bucket index "index";
 * Updates the cache in case when "block" is already present in the block.
 */
void UUIDColumn::cacheBucketBlock(
    const BucketIndex index,
    const SharedBucketBlock block) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(index < totalBucketsCount());
    assert(block != nullptr);
#endif

    mReadWriteCache.insert(make_pair(index, block));
}

const pair<AbstractRecordsHandler::RecordNumber*, AbstractRecordsHandler::RecordsCount>
UUIDColumn::recordNumbersAssignedToUUID(
    const NodeUUID &uuid,
    const bool clearReadCacheOnSuccess) {

    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto bucket = getBucketBlock(bucketIndex);

    try {
        auto record = bucket->recordByUUID(uuid);
        if (clearReadCacheOnSuccess) {
            clearReadCache();
        }

        return make_pair(record->recordNumbers(), record->count());

    } catch (IndexError &e) {
        // There is no such uuid in the bucket block.
        // As a result - there is no records assigned to this uuid.
        return make_pair(nullptr, 0);
    }
}

UUIDColumn::FileHeader UUIDColumn::loadFileHeader() const {
    FileHeader header;
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fread(&header, sizeof(header), 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDColumn::loadFileHeader: "
                "can't load file header.");
    }
    return header;
}

void UUIDColumn::updateFileHeader(
    const FileHeader *header) const {

    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fwrite(header, sizeof(FileHeader), 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDColumn::updateFileHeader: "
                "can't write file header to the disk.");
    }
    syncLowLevelOSBuffers();
}

void UUIDColumn::initDefaultFileHeader() {
    FileHeader header(1, FileHeader::READ_WRITE);
    updateFileHeader(&header);
}

void UUIDColumn::initFromFileHeader() {
    FileHeader header = loadFileHeader();

    if (header.version != 1) {
        throw ValueError(
            "UUIDColumn::initFromFileHeader: "
                "unexpected file version occurred.");
    }
}

void UUIDColumn::commitCachedBlocks() {
    // todo: check if file is not read only.

    try {
        for (auto& kv : mReadWriteCache) {
            const auto bucketIndex = kv.first;
            const auto block = kv.second;

            writeBlock(bucketIndex, block);
        }
        syncLowLevelOSBuffers();

    } catch (Exception &e) {
        // ToDo: make file read only.
        throw e;
    }

    // note: memory will be freed by the shared pointers;
    mReadWriteCache.clear();
}

/*!
 * Atomically writes "block" (bucket) to the disk and updates blocks index.
 *
 * The block would be reallocated on the disk.
 * Old block should be removed by the vacuum().
 *
 *
 * Throws IOError;
 * Throws MemoryError.
 */
void UUIDColumn::writeBlock(
    const BucketIndex bucketIndex,
    const SharedBucketBlock block) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bucketIndex < totalBucketsCount());
#endif

    if (!block->isModified()) {
        return;
    }

    // todo: check if file is not read only.

    if (block->recordsCount() == 0) {
        removeBlockFromRecordNumbersIndex(block);

        // The block is empty.
        // Only blocks index must be updated (settigns address to zero).
        BucketsIndexRecord indexRecord;
        indexRecord.bytesCount = 0;
        indexRecord.offset = kNoBucketAddressValue;

        seekToBucketIndexRecord(bucketIndex);
        if (fwrite(&indexRecord, sizeof(indexRecord), 1, mFileDescriptor) != 1 &&
            fwrite(&indexRecord, sizeof(indexRecord), 1, mFileDescriptor) != 1) {
            throw IOError(
                "UUIDColumn::writeBlock: "
                    "cant update blocks index.");
        }

    } else {
        // Block is not empty an should be reallocated.

        // In case if some records was removed from the block - record numbers index should be updated.
        // But it is impossible in this context to know what records has been removed (if any),
        // so the records index must be rewitten for all the records:
        //  obsolete block records should be removed from the index,
        //  and new block should be indexed.

        // todo: there is a more efficient way to this
        // Only some records delta should be removed from the index:
        // (only record that are not present in current block, but are present in obsolete block)

        // This operation is not atomic.
        // In case of error - index would be broken.
        // But index may be recreated after transaction failure:
        // this is a responsiblity of transactions handler, and not the column.
        // todo: add reindexAllRecordNumbers()

        try {
            auto obsoleteBlock = readBucket(bucketIndex);
            removeBlockFromRecordNumbersIndex(obsoleteBlock);

        } catch (NotFoundError &) {
            // There is no previous block record for this bucket.
        }


        // New block will be written to the end of file.
        BucketsIndexRecord indexRecord;
        if (fseek(mFileDescriptor, 0, SEEK_END) != 0){
            throw IOError(
                "UUIDColumn::writeBlock: "
                    "cant seek to the end of the file.");
        }
        indexRecord.offset = ftell(mFileDescriptor);

        // Writing block to the disk
        auto serializationResult = block->serializeToBytes();
        if (fwrite(serializationResult.first.get(), serializationResult.second, 1, mFileDescriptor) != 1 &&
            fwrite(serializationResult.first.get(), serializationResult.second, 1, mFileDescriptor) != 1) {
            throw IOError(
                "UUIDColumn::writeBlock: "
                    "cant write bucket block to the disk.");
        }

        // Updating the blocks index
        indexRecord.bytesCount = serializationResult.second;
        seekToBucketIndexRecord(bucketIndex);
        if (fwrite(&indexRecord, sizeof(indexRecord), 1, mFileDescriptor) != 1 &&
            fwrite(&indexRecord, sizeof(indexRecord), 1, mFileDescriptor) != 1) {
            throw IOError(
                "UUIDColumn::writeBlock: "
                    "cant update blocks index.");
        }

        reindexBlockInRecordNumbersIndex(block, indexRecord);
    }
}

/*!
 * Initializes buckets index, according to the received buckets count index.
 *
 *
 * Throws IOError.
 */
void UUIDColumn::initBucketsIndex() {
    BucketsIndexRecord emptyRecord;
    emptyRecord.offset = kNoBucketAddressValue;
    emptyRecord.bytesCount = 0;

    // Seek after the file header.
    fseek(mFileDescriptor, sizeof(FileHeader), SEEK_SET);

    const auto bucketsCount = totalBucketsCount();
    for (size_t i = 0; i < bucketsCount; ++i) {
        if (fwrite(&emptyRecord, sizeof(emptyRecord), 1, mFileDescriptor) != 1) {
            throw IOError(
                "UUIDColumn::initBucketsIndex: "
                    "can't write buckets index to the disk.");
        }
    }
    syncLowLevelOSBuffers();
}

/*
 * Removes from the cache all blocks, that wasn't modified by the write oprations.
 * Modified blocks should be committed (see: commitCachedBlocks());
 *
 *
 * This checkFileCreation is useful in scenarios when several (thousands?) read operations
 * should be performed at once (for example, through transaction, or path searching operation).
 * To prevent heavy dik usage - block, that was read once, will be cached,
 * and consequent read operations will be made with it's copy in memory.
 *
 * But when the operation is done - cache should be shrinked back,
 * to prevent redundant memory usage.
 *
 * It is the developer decision when to shrink the read cache.
 * (it may be important to have control on this in transactions,
 * that are speed-critical).
 */
void UUIDColumn::clearReadCache() {
    auto cacheCopy = mReadWriteCache;

    // Iteration is done on the cache copy
    // to prevent iterator invalidation after record erasing.
    for (auto& kv : cacheCopy) {
        const auto bucketIndex = kv.first;
        const auto block = kv.second;

        if (block->isModified()) {
            // Record is modified and should be committed.
            // It cant be removed from the cache.
            continue;
        }

        mReadWriteCache.erase(bucketIndex);
    }
}

/*!
 * Removes all record numbers that are in the block from the record numbers index.
 * WARN: this operation is not atomic.
 *
 *
 * Throws IOError;
 */
void UUIDColumn::removeBlockFromRecordNumbersIndex(
    const SharedBucketBlock block) {

    auto records = block->records();
    for (size_t r=0; r < block->recordsCount(); ++r) {
        auto record = records + r;
        auto recordNumbers = record->recordNumbers();

        for (size_t rn=0; rn < record->count(); ++rn) {

            // Current C Filesystem API allows only uint32 file offsets.
            // So it's OK, no type owerflow is possible.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
            mRecordsIndex.remove(recordNumbers[rn], false);
#pragma clang diagnostic pop
        }
    }
    mRecordsIndex.commit();
}

/*!
 * Inserts all record numbers that are in the block to the record numbers index.
 * WARN: this operation is not atomic.
 *
 *
 * Throws IOError;
 */
void UUIDColumn::reindexBlockInRecordNumbersIndex(
    const SharedBucketBlock block,
    const BucketsIndexRecord recordIndex) {

    auto records = block->records();
    for (size_t r=0; r < block->recordsCount(); ++r) {
        auto record = records + r;
        auto recordNumbers = record->recordNumbers();

        for (size_t rn=0; rn < record->count(); ++rn) {

            // Current C Filesystem API allows only uint32 file offsets.
            // So it's OK, no type owerflow is possible.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
            mRecordsIndex.set(recordNumbers[rn], recordIndex.offset, false);
#pragma clang diagnostic pop
        }
    }
    mRecordsIndex.commit();
}


}
}

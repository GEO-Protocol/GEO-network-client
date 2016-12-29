#include "UUIDColumn.h"


namespace db {
namespace fields {


UUIDColumn::UUIDColumn(
    const fs::path &path,
    const uint8_t pow2BucketsCountIndex):

    AbstractFileDescriptorHandler(path / fs::path("data.bin")),
    mRecordsIndex(path / fs::path("index.dat")),
    mPow2BucketsCountIndex(pow2BucketsCountIndex){

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(path.filename() == fs::path("."));
    assert(! path.has_extension());
#endif

    if (pow2BucketsCountIndex == 0 || pow2BucketsCountIndex > 16) {
        throw ValueError(
            "UUIDColumn::UUIDColumn: "
                "\"pow2BucketsCountIndex\" can't be equal to 0 or greater than 16.");
    }

    open(kWriteAccessMode);
}


UUIDColumn::FileHeader::FileHeader(
    const uint16_t version,
    const uint8_t state):

    version(version),
    state(state) {}

UUIDColumn::FileHeader::FileHeader():
    version(1),
    state(READ_WRITE) {}


/*!
 * Attempts to assign "recN" to the "uuid".
 * If "commit" is true - internal read/write cache would be synced to the disk.
 *
 * Throws "ConflictError" in case, when equal record is already present in the block.
 * Throws "OverflowError" in case when block is full and no more records may be inserted.
 *
 * Throws "MemoryError" in case when there is not enough memory for the operation.
 * Throws IOError in case when i/o error occured.
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

void UUIDColumn::open(const char *accessMode) {
    AbstractFileDescriptorHandler::open(accessMode);

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

    // BucketBlock will take ownership on "blockData".
    return SharedBucketBlock(new BucketBlock(blockData));
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
                "can't write header to the disk.");
    }
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "UUIDColumn::updateFileHeader: "
                "can't sync buffers with the disk.");
    }
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
    for (auto& kv : mReadWriteCache) {
        const auto bucketIndex = kv.first;
        const auto block = kv.second;

        try {
            writeBlock(bucketIndex, block);
        } catch (Exception &e) {
            // ToDo: make file read only.
            throw e;
        }
    }

    // note: memory will be freed by the shard pointers;
    mReadWriteCache.clear();
}

/*!
 * Atomically writes "block" to the bucket with index "bucketIndex".
 *
 * Throws IOError in case of any error.
 * Throws MemoryError.
 */
void UUIDColumn::writeBlock(
    const BucketIndex bucketIndex,
    const SharedBucketBlock block) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bucketIndex < totalBucketsCount());
#endif

    if (!block->isModified() || block->recordsCount() == 0) {
        return;
    }

    // todo: check if file is not read only.


    // New block will be written to the end of file.
    if (fseek(mFileDescriptor, 0, SEEK_END) != 0){
        throw IOError(
            "UUIDColumn::writeBlock: "
                "cant seek to the end of the file.");
    }

    BucketsIndexRecord mapIndexRecord;
    mapIndexRecord.offset = ftell(mFileDescriptor);


    // Writing block to the disk
    auto serializationResult = block->serializeToBytes();
    if (fwrite(serializationResult.first.get(), serializationResult.second, 1, mFileDescriptor) != 1 &&
        fwrite(serializationResult.first.get(), serializationResult.second, 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDColumn::writeBlock: "
                "cant write bucket block to the disk.");
    }
    syncLowLevelOSBuffers();


    // Updating the blocks index
    mapIndexRecord.bytesCount = serializationResult.second;

    seekToBucketIndexRecord(bucketIndex);
    if (fwrite(&mapIndexRecord, sizeof(mapIndexRecord), 1, mFileDescriptor) != 1 &&
        fwrite(&mapIndexRecord, sizeof(mapIndexRecord), 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDColumn::writeBlock: "
                "cant update blocks index.");
    }
    syncLowLevelOSBuffers();
}

/*!
 * Initializes buckets index, according to received buckets count index.
 *
 * May thrown IOError.
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
            // Block is modified and should be committed.
            // It cant be removed from the cache.
            continue;
        }

        mReadWriteCache.erase(bucketIndex);
    }
}


}
}

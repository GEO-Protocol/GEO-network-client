#include "UUIDMapColumn.h"


namespace db {
namespace fields {


UUIDMapColumn::UUIDMapColumn(
    const char *path,
    const uint8_t pow2BucketsCountIndex):

    AbstractFileDescriptorHandler(path, "data.bin"),
    mRecordsIndex(path, "idx.bin"),
    mPow2BucketsCountIndex(pow2BucketsCountIndex){

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(path != nullptr);
#endif

    if (pow2BucketsCountIndex == 0 || pow2BucketsCountIndex > 16) {
        throw ValueError(
            "UUIDMapColumn::UUIDMapColumn: "
                "\"pow2BucketsCountIndex\" can't be equal to 0 or greater than 16.");
    }

    open(kWriteAccessMode);
}


UUIDMapColumn::FileHeader::FileHeader(
    const uint16_t version,
    const uint8_t state,
    const uint16_t bucketsCount):

    version(version),
    state(state) {}

UUIDMapColumn::FileHeader::FileHeader():
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
void UUIDMapColumn::set(
    const NodeUUID &uuid,
    const BucketRecordNumber recN,
    const bool commit) {

    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto bucketBlock = getBucketBlock(bucketIndex);

    bucketBlock->insert(uuid, recN);
    cacheBucketBlock(bucketIndex, bucketBlock);

    if (commit) {
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
const bool UUIDMapColumn::remove(
    const NodeUUID &uuid,
    const UUIDMapColumn::BucketRecordNumber recN,
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

//const BucketRecordNumber UUIDMapColumn::bucketRecordIndexByUUID(
//    const NodeUUID &uuid) const {
//
//    auto bucketIndex = bucketIndexByNodeUUID(uuid);
//    auto bucket = getBucketBlock(bucketIndex);
//
//    return bucket->recordIndexByUUID(uuid);
//}

/*!
 * Writes empty header to the file.
 * See the BucketBlockDescriptor class docs for the details.
 */
//void UUIDMapColumn::initFileHeader() {
//    // Initialising memory buffer for empty header record.
//    byte *emptyBucketsHeaderRecord = (byte *)malloc(kBucketsHeaderRecordSize);
//    if (emptyBucketsHeaderRecord == nullptr) {
//        throw MemoryError(
//            "UUIDMapColumn::initFileHeader "
//                "can't allocate enough memory for empty buckets header record.");
//    }
//    memset(emptyBucketsHeaderRecord, 0, kBucketsHeaderRecordSize);
//
//    // Writing empty record header for each one bucket.
//    fseek(mFileDescriptor, 0, SEEK_SET);
//
//    const auto bucketsCount = totalBucketsCount();
//    for (size_t i = 0; i < bucketsCount; ++i) {
//        auto recordsWritten = fwrite(
//            emptyBucketsHeaderRecord, kBucketsHeaderRecordSize, 1, mFileDescriptor);
//
//        if (recordsWritten != 1) {
//            free(emptyBucketsHeaderRecord);
//
//            throw IOError(
//                "UUIDMapColumn::initFileHeader: "
//                    "can't write empty bucket header to the disk.");
//        }
//    }
//
//    // Seems to be OK
//    free(emptyBucketsHeaderRecord);
//}

void UUIDMapColumn::open(const char *accessMode) {
    AbstractFileDescriptorHandler::open(accessMode);

    if (fileSize() == 0) {
        initDefaultFileHeader();
        initBucketsIndex();

    } else {
        initFromFileHeader();
    }
}

const size_t UUIDMapColumn::totalBucketsCount() const {
    return (size_t)pow(double(2), mPow2BucketsCountIndex);
}

/*!
 * @param u - uuid for which the bucket index should be calculated.
 * @return index (not address) of the bucket, that contains the "u".
 * To get the bucket - this index should be multiplied by the record size.
 */
const UUIDMapColumn::BucketIndex UUIDMapColumn::bucketIndexByNodeUUID(const NodeUUID &u) const {
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

//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wconversion"
//    const byte middleOffset = (NodeUUID::kUUIDLength - mPow2BucketsCountIndex) / 2;
//#pragma clang diagnostic pop

    vector<uint8_t> bytes;
    bytes.reserve(NodeUUID::kUUIDLength);

    for (uint8_t i=NodeUUID::kUUIDLength-1; i>=0; i--){
        bytes.push_back(u.data[i]);
    }

    boost::multiprecision::uint128_t index;
    boost::import_bits_fast(index, bytes.begin(), bytes.end());

    return BucketIndex(index / totalBucketsCount());
}

BucketBlock* UUIDMapColumn::getBucketBlock(const UUIDMapColumn::BucketIndex index) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(index < totalBucketsCount());
#endif

    if (mReadWriteCache.count(index) > 0) {
        return mReadWriteCache.at(index);

    } else {
        // Cache doesn't contains the block.

        try {
            auto bucketBlock = readBucket(index);
            cacheBucketBlock(index, bucketBlock);
            return bucketBlock;

        } catch (NotFoundError) {
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
                    "UUIDMapColumn::getBucketBlock: "
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
BucketBlock* UUIDMapColumn::readBucket(BucketIndex index) const {
//
//#ifdef INTERNAL_ARGUMENTS_VALIDATION
//    assert(index < totalBucketsCount());
//#endif
//
//    struct {
//        BucketBlockAddress address;
//        BucketRecordsCount recordsCount;
//    } header;
//
//    fseek(mFileDescriptor, index * kBucketsHeaderRecordSize, SEEK_SET);
//    fread(&header, kBucketsHeaderRecordSize, 1, mFileDescriptor);
//    if (header.address == kNoBucketAddressValue) {
//        // There is no block assigned to this bucket.
//        throw NoBucketBlockError(
//            "UUIDMapColumn::readBucket: "
//                "there is no bucket block associated with this bucket.");
//    }
//    return readBucketBlock(header.address, header.recordsCount);
//}
//
///*!
// * Tries to read bucket block from the disk.
// * Returns it in case of success;
// *
// * Throws "MemoryError"
// *
// * @param address - specifies address of the block in the file.
// * @param recordsCount - specifies how many records should be read starting from "address".
// */
//BucketBlock* UUIDMapColumn::readBucketBlock(const UUIDMapColumn::BucketBlockAddress address,
//                                             const UUIDMapColumn::BucketRecordsCount recordsCount) const {
//
//#ifdef INTERNAL_ARGUMENTS_VALIDATION
//    assert(address >= kBucketsHeaderRecordSize * totalBucketsCount());
//    assert(recordsCount > 0);
//#endif
//
//    byte *blockData = (byte*) malloc(recordsCount * kBucketsHeaderRecordSize);
//    if (blockData == nullptr) {
//        throw MemoryError(
//            "UUIDMapColumn::readBucketBlock: "
//                "can't allocate memory for the bucket block.");
//    }
//
//    fseek(mFileDescriptor, address, SEEK_SET);
//    auto recordsRead = fread(
//        blockData, recordsCount * kBucketsHeaderRecordSize, 1, mFileDescriptor);
//
//    if (recordsRead != 1) {
//        free(blockData);
//        throw MemoryError(
//            "UUIDMapColumn::readBucketBlock: "
//                "can't dataOffset block from the disk.");
//    }
//
//    // Note:
//    // BucketBlock will take ownership on "blockData".
//    return new BucketBlock((BucketBlockRecord*)blockData, recordsCount);
}

/*!
 * Caches the "block" by the bucket index "index";
 * Updates the cache in case when "block" is already present in the block.
 */
void UUIDMapColumn::cacheBucketBlock(const BucketIndex index, BucketBlock *block) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(index < totalBucketsCount());
    assert(block != nullptr);
#endif

    mReadWriteCache.insert(make_pair(index, block));
}

const pair<AbstractRecordsHandler::RecordNumber *, AbstractRecordsHandler::RecordsCount>
UUIDMapColumn::recordNumbersAssignedToUUID(const NodeUUID &uuid) {

    auto bucketIndex = bucketIndexByNodeUUID(uuid);
    auto bucket = getBucketBlock(bucketIndex);

    try {
        auto record = bucket->recordByUUID(uuid);
        return make_pair(record->recordNumbers(), record->count());

    } catch (IndexError &e) {
        // There is no such uuid in the bucket block.
        // As a result - there is no records assigned to this uuid.
        return make_pair(nullptr, 0);
    }
}

UUIDMapColumn::FileHeader UUIDMapColumn::loadFileHeader() const {
    FileHeader header;
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fread(&header, sizeof(header), 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDMapColumn::loadFileHeader: "
                "can't load file header.");
    }
    return header;
}

void UUIDMapColumn::updateFileHeader(
    const FileHeader *header) const {

    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fwrite(header, sizeof(FileHeader), 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDMapColumn::updateFileHeader: "
                "can't write header to the disk.");
    }
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "UUIDMapColumn::updateFileHeader: "
                "can't sync buffers with the disk.");
    }
}

void UUIDMapColumn::initDefaultFileHeader() {
    FileHeader header(1, FileHeader::READ_WRITE, 0);
    updateFileHeader(&header);

    mState = (FileHeader::States)header.state;
}

void UUIDMapColumn::initFromFileHeader() {
    FileHeader header = loadFileHeader();

    if (header.version != 1) {
        throw ValueError(
            "UUIDMapColumn::initFromFileHeader: "
                "unexpected file version occurred.");
    }

    mState = (FileHeader::States)header.state;
}

void UUIDMapColumn::commitCachedBlocks() {
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

    // Blocks are written successfully.
    // Invalidate the cache.
    for (auto iterator = mReadWriteCache.begin(); iterator != mReadWriteCache.end(); iterator++) {
        const auto block = (*iterator).second;
        delete block;
    }
    mReadWriteCache.clear();
}

/*!
 * Atomically writes "block" to the bucket with index "bucketIndex".
 *
 * Throws IOError in case of any error.
 * Throws MemoryError.
 */
void UUIDMapColumn::writeBlock(
    const BucketIndex bucketIndex,
    BucketBlock *block) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bucketIndex < totalBucketsCount());
    assert(block != nullptr);
#endif

    if (! block->isModified()) {
        return;
    }

    if (block->recordsCount() == 0) {
        return;
    }

    // todo: check if file is not read only.


    // New block will be written to the end of file.
    fseek(mFileDescriptor, 0, SEEK_END);

    BucketsIndexRecord mapIndexRecord;
    mapIndexRecord.offset = ftell(mFileDescriptor);
    mapIndexRecord.bytesCount = 0;


    // Each block should be prefixed with "totalRecordsCount" field,
    // to be able to read only UUIDs block (see further).
    const auto kRecordsCount = block->recordsCount();
    if (fwrite(&kRecordsCount, sizeof(kRecordsCount), 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDMapColumn::writeBlock: "
                "can't write total recordNumbers count.");
    }
    mapIndexRecord.bytesCount += sizeof(kRecordsCount);

    // Write all UUIDs one by one into continuous block.
    auto record = block->records();
    for (size_t i=0; i < kRecordsCount; ++i) {
        auto uuid = record->uuid();
        if (fwrite(uuid.data, NodeUUID::kUUIDLength, 1, mFileDescriptor) != 1) {
            throw IOError(
                "UUIDMapColumn::writeBlock: "
                    "can't write record uuid to the disk.");
        }

        ++record;
    }
    mapIndexRecord.bytesCount += NodeUUID::kUUIDLength * kRecordsCount;

    // Write all records numbers that are in the record,
    // one by one into continuous block.
    RecordNumbersIndex::DataOffset recordsNumbersOffset =
        (RecordNumbersIndex::DataOffset) ftell(mFileDescriptor);

    record = block->records();
    for (RecordsCount recordIndex=0; recordIndex < kRecordsCount; ++recordIndex) {

        // Writing total records numbers count to the file
        // to be ale to read whole the record data at once.
        const auto recordNumbersCount = record->count();
        if (fwrite(&recordNumbersCount, sizeof(recordNumbersCount), 1, mFileDescriptor) != 1) {
            throw IOError(
                "UUIDMapColumn::writeBlock: "
                    "can't write record numbers count to the disk.");
        }

        const auto recordData = record->data();
        const auto data = recordData.first;
        const auto bytesCount = recordData.second;
        if (fwrite(data, bytesCount, 1, mFileDescriptor) != 1) {
            throw IOError(
                "UUIDMapColumn::writeBlock: "
                    "can't write record to the disk.");
        }

        // Updating of records number index.
        // Each record number in the index should be
        // re-assigned with the new position of it's uuid.
        RecordNumbersIndex::DataOffset newOffset =
            recordsNumbersOffset + (recordIndex * NodeUUID::kUUIDLength);

        RecordNumber *nextRecN = record->recordNumbers();
        for (size_t recNumberIndex=0; recNumberIndex < record->count(); ++recNumberIndex) {
            mRecordsIndex.set(*nextRecN, newOffset);
        }

        // Move to the next record
        ++record;
    }


    // Update block address in the map index.
    const size_t indexRecordOffset = (bucketIndex * sizeof(BucketsIndexRecord))
        + sizeof(FileHeader);

    fseek(mFileDescriptor, indexRecordOffset, SEEK_SET);
    if (fwrite(&mapIndexRecord, sizeof(mapIndexRecord), 1, mFileDescriptor) != 1) {
        throw IOError(
            "UUIDMapColumn::writeBlock: "
                "can't update map index.");
    }


    // Sync buffers
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "UUIDMapColumn::writeBlock: "
                "can't sync user-space buffers.");
    }
}

/*!
 * Initializes buckets index, according to received buckets count index.
 *
 * May thrown IOError.
 */
void UUIDMapColumn::initBucketsIndex() {
    BucketsIndexRecord emptyRecord;
    memset(&emptyRecord, 0, sizeof(emptyRecord));

    fseek(mFileDescriptor, sizeof(FileHeader), SEEK_SET);

    const auto bucketsCount = totalBucketsCount();
    for (size_t i = 0; i < bucketsCount; ++i) {
        if (fwrite(&emptyRecord, sizeof(emptyRecord), 1, mFileDescriptor) != 1) {
            throw IOError(
                "UUIDMapColumn::initBucketsIndex: "
                    "can't write empty buckets index to the disk.");
        }
    }

    // Sync buffers
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "UUIDMapColumn::initBucketsIndex: "
                "can't sync user-space buffers.");
    }
}


}
}

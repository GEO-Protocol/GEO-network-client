#ifndef GEO_NETWORK_CLIENT_UUIDMAP_H
#define GEO_NETWORK_CLIENT_UUIDMAP_H

#include "../../common/AbstractFileDescriptorHandler.h"
#include "../../common/AbstractRecordsHandler.h"

#include "BucketBlock.h"

#include <malloc.h>
#include <map>
#include <cmath>


namespace db {
namespace fields{


using namespace std;
using namespace db::fields::uuid_map;

/*!
 * Indicates that there is no BucketBlock is assigned with the bucket.
 */
class NoBucketBlockError: public Exception {
    using Exception::Exception;
};


class UUIDMapStorage: protected AbstractFileDescriptorHandler,
                      protected AbstractRecordsHandler {
public:
    typedef uint16_t BucketIndex;
    typedef uint32_t BucketBlockAddress;

    typedef uint32_t BucketRecordsCount;
    typedef BucketRecordsCount BucketRecordNumber;
    typedef BucketRecordNumber BucketRecordOffset;

public:
    explicit UUIDMapStorage(
        const char *filename,
        const char *path,
        const uint8_t pow2BucketsCountIndex);

    void set(
        const NodeUUID &uuid,
        const RecordNumber recN);

    const bool remove(
        const NodeUUID &uuid,
        const RecordNumber recN);

    const pair<RecordNumber*, RecordsCount> recordNumbersAssignedToUUID(
        const NodeUUID &uuid);

protected:
    void initFileHeader();
    virtual const size_t totalBucketsCount() const;
    virtual const BucketIndex bucketIndexByNodeUUID(
        const NodeUUID &u) const;

    void open(
        const char *accessMode);

    BucketBlock* getBucket(
        const BucketIndex index);

    BucketBlock* readBucket(
        BucketIndex index) const;

    BucketBlock* readBucketBlock(
        const BucketBlockAddress address,
        const BucketRecordsCount recordsCount) const;

    void cacheBucketBlock(const BucketIndex index,
                          BucketBlock *block);

protected:
    /*!
     * Bucket blocks are stored in the file after the header.
     * Header - is the special block of fixed size,
     * that is written from the beginning of the file.
     *
     * Header maps buckets to their current blocks.
     * It contains address of the block, and records count,
     * that are stored in this block.
     *
     * In case when new record is adding to the block -
     * whole the block reallocates to the new position in the file.
     *
     * Block updates are non-atomic.
     * Instead of blocks updates block reallocation is used.

     * (with header address updating, which is atomic)
     * The old block will also remain in the file.
     * It will be removed from the file by the vacuum() procedure.
     */
    static constexpr const uint8_t kBucketsHeaderRecordSize =
        sizeof(BucketRecordOffset) +
        sizeof(BucketRecordsCount);

    static constexpr const uint8_t kUUIDLength = 16;
    static constexpr const BucketBlockAddress kNoBucketAddressValue = 0;

protected:
    uint8_t mPow2BucketsCountIndex;

    map<BucketIndex, BucketBlock*> mBlocksCache;
};


} // namespace fields
} // namespace db





#endif //GEO_NETWORK_CLIENT_UUIDMAP_H

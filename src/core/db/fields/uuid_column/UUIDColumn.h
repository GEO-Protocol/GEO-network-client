#ifndef GEO_NETWORK_CLIENT_UUIDMAP_H
#define GEO_NETWORK_CLIENT_UUIDMAP_H

#include "internal/BucketBlock.h"

#include "../common/AbstractFileDescriptorHandler.h"
#include "../common/AbstractRecordsHandler.h"
#include "../common/RecordNumbersIndex.h"
#include "../../../common/NodeUUID.h"
#include "../../../common/exceptions/NotFoundError.h"

#include <boost/multiprecision/cpp_int.hpp>

#include <map>
#include <malloc.h>


namespace db {
namespace fields{


using namespace std;
using namespace db::fields::uuid_map;

typedef shared_ptr<BucketBlock> SharedBucketBlock;


// todo: describe file format
class UUIDColumn:
    protected AbstractFileDescriptorHandler,
    protected AbstractRecordsHandler {

public:
    typedef uint16_t BucketIndex;
    typedef uint32_t BucketBlockAddress;

    typedef uint32_t BucketRecordsCount;
    typedef BucketRecordsCount BucketRecordNumber;
    typedef int64_t BucketOffset;

public:
    explicit UUIDColumn(
        const fs::path &path,
        const uint8_t pow2BucketsCountIndex);

    void set(
        const NodeUUID &uuid,
        const RecordNumber recN,
        const bool sync=false);

    const bool remove(
        const NodeUUID &uuid,
        const RecordNumber recN,
        const bool commit=false);

    const pair<RecordNumber*, RecordsCount>
        recordNumbersAssignedToUUID(
            const NodeUUID &uuid,
            const bool clearReadCacheOnSuccess=false);

    void commitCachedBlocks();
    void clearReadCache();

protected:
    static constexpr const BucketBlockAddress kNoBucketAddressValue = 0;

protected:
    struct FileHeader {
        enum States {
            READ_WRITE = 0,
            READ_ONLY  = 1,
        };

        explicit FileHeader();
        explicit FileHeader(
            const uint16_t version,
            const uint8_t  state);

        // Version is useful in case when data migration is needed.
        // Current scheme version is v1;
        uint16_t version;

        // todo: add logic to make file read only.
        uint8_t state;
    };

    struct BucketsIndexRecord {
        // Specifies current offset of the block with bucket info.
        BucketOffset offset;

        // Specifies how long (in bytes) the bucket block is.
        uint32_t bytesCount;
    };

    // Specifies in k**2 format how many buckets would be created on the disk.
    // This member should be initialized with value from 1 to 16.
    //
    // For example, if this member would be initialized with value 2 -
    // then there would be created 4 buckets on the the disk,
    // for the value 4 - 16, etc.
    uint8_t mPow2BucketsCountIndex;

    map<BucketIndex, SharedBucketBlock> mReadWriteCache;
    RecordNumbersIndex mRecordsIndex;

protected:
    virtual const size_t totalBucketsCount() const;
    virtual const BucketIndex bucketIndexByNodeUUID(
        const NodeUUID &u) const;

    FileHeader loadFileHeader() const;
    void updateFileHeader(
        const FileHeader *header) const;
    void initDefaultFileHeader();
    void initFromFileHeader();
    void initBucketsIndex();

    inline void seekToBucketIndexRecord(
        BucketIndex index) const;

    SharedBucketBlock getBucketBlock(
        const BucketIndex index);
    SharedBucketBlock readBucket(
        const BucketIndex index) const;
    SharedBucketBlock readBlock(
        const BucketsIndexRecord indexRecord) const;

    void writeBlock(
        const BucketIndex bucketIndex,
        const SharedBucketBlock block);

    void cacheBucketBlock(
        const BucketIndex index,
        const SharedBucketBlock block);


    void open(
        const char *accessMode);
};


} // namespace fields
} // namespace db





#endif //GEO_NETWORK_CLIENT_UUIDMAP_H

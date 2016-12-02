#ifndef GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H
#define GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H

#include "Types.h"
#include "../../../common/exceptions/MemoryError.h"
#include "../../../common/exceptions/OverflowError.h"
#include "../../../common/exceptions/ConflictError.h"

#include <cstring>
#include <limits>
#include "malloc.h"


namespace db {
namespace routing_tables {


using namespace std;

/*
 * Bucket block should map uuid's to their record. numbers.
 * For example:
 *    uuid1 -> recNo1
 *    uuid2 -> recNo2
 *    uuid3 -> recNo3
 *
 * But, duplicates may appear in the block,
 * and the map may be as shown next:
 *    uuid1 -> recNo1
 *    uuid1 -> recNo2
 *    uuid1 -> recNo3
 *    uuid2 -> recNo4
 *    uuid2 -> recNo5
 *
 * Obviously, that it would be much more efficiently to store
 * this structure in the next way:
 *    uuid1 -> [recNo1, recNo2, recNo3]
 *    uuid2 -> [recNo4, recNo5]
 *
 * This class implements exactly that structure.
 */
class BucketBlockRecord {
    friend class BucketBlockRecordTests;

public:
    BucketBlockRecord(const NodeUUID &uuid);
    BucketBlockRecord(const NodeUUID &uuid,
                      RecordsCount recordsNumbersCount,
                      RecordNumber *recordsNumbers);
    ~BucketBlockRecord();

    void insert(const RecordNumber recNo);
    bool remove(const RecordNumber recNo);

    const byte* data() const;

private:
    const NodeUUID &mUUID;

    // Each record stores pair of <uuid> -> [recNo, recNo, recNo, ...]
    // This member specifies how much elements are in the list of records numbers.
    RecordsCount mRecordsNumbersCount;
    RecordNumber *mRecordsNumbers;
};


} // namespace routing_tables
} // namespace db


#endif //GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H

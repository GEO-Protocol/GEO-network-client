#ifndef GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H
#define GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H

#include "Types.h"
#include "../../../common/NodeUUID.h"
#include "../../../common/exceptions/MemoryError.h"
#include "../../../common/exceptions/OverflowError.h"
#include "../../../common/exceptions/ConflictError.h"
#include "../../../common/exceptions/IndexError.h"

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

    void insert(const RecordNumber recN);
    bool remove(const RecordNumber recN);

    const byte* data() const;
    const NodeUUID& uuid() const;
    const bool isModified() const;

protected:
    const RecordsCount indexOf(const RecordNumber recN);

protected:
    const NodeUUID &mUUID;

    RecordsCount mRecordsNumbersCount;
    RecordNumber *mRecordsNumbers;
    bool mHasBeenModified;
};


} // namespace routing_tables
} // namespace db


#endif //GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H

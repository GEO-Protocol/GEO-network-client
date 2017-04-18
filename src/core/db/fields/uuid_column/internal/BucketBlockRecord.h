#ifndef GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H
#define GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H


#include "../../common/AbstractRecordsHandler.h"
#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/exceptions/MemoryError.h"
#include "../../../../common/exceptions/OverflowError.h"
#include "../../../../common/exceptions/ConflictError.h"
#include "../../../../common/exceptions/IndexError.h"

#include <cstring>
#include <limits>

#include "stdlib.h"


namespace db {
namespace fields {
namespace uuid_map {


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
class BucketBlockRecord:
    protected AbstractRecordsHandler {
    friend class BucketBlockTests;
    friend class BucketBlockRecordTests;

public:
    BucketBlockRecord(
        const NodeUUID &uuid);
    BucketBlockRecord(
        byte *data);

    ~BucketBlockRecord();

    void insert(
        const RecordNumber recN);
    bool remove(
        const RecordNumber recN);

    const RecordsCount count() const;
    RecordNumber* recordNumbers() const;

    const pair<void*, size_t> data() const;
    const NodeUUID& uuid() const;
    const bool isModified() const;

protected:
    const RecordsCount indexOf(
        const RecordNumber recN);

protected:
    NodeUUID mUUID;
    RecordNumber *mRecordsNumbers;

    RecordsCount mRecordsNumbersCount;
    bool mHasBeenModified;
};


} // namespace uuid_map
} // namespace fields
} // namespace db

#endif //GEO_NETWORK_CLIENT_BUCKETBLOCKRECORD_H

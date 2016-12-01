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

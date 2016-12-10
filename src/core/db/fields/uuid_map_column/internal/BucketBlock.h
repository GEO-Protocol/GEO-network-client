#ifndef GEO_NETWORK_CLIENT_BUCKETBLOCKDESCRIPTOR_H
#define GEO_NETWORK_CLIENT_BUCKETBLOCKDESCRIPTOR_H

#include "BucketBlockRecord.h"
#include "UUIDMapStorage.h"


namespace db {
namespace fields {
namespace uuid_map {


using namespace std;


class BucketBlock: protected AbstractRecordsHandler {
    friend class BucketBlockTests;

public:
    explicit BucketBlock();
    explicit BucketBlock(
        BucketBlockRecord *records,
        const RecordNumber recordsCount);

    ~BucketBlock();

    void insert(
        const NodeUUID &uuid,
        const RecordNumber recN);

    bool remove(
        const NodeUUID &uuid,
        const RecordNumber recN);

    BucketBlockRecord* recordByUUID(
        const NodeUUID &uuid) const;

    const RecordNumber recordIndexByUUID(
        const NodeUUID &uuid) const;

    const bool isModified() const;

protected:
    BucketBlockRecord* createRecord(
        const NodeUUID &uuid);
    void dropRecord(BucketBlockRecord *record);

protected:
    BucketBlockRecord *mRecords;
    RecordNumber mRecordsCount;
    bool mHasBeenModified;
};

} // namespace uuid_map
} // namespace fields
} // namespace db

#endif //GEO_NETWORK_CLIENT_BUCKETBLOCKDESCRIPTOR_H

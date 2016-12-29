#ifndef GEO_NETWORK_CLIENT_BUCKETBLOCKDESCRIPTOR_H
#define GEO_NETWORK_CLIENT_BUCKETBLOCKDESCRIPTOR_H


#include "BucketBlockRecord.h"
#include "../../common/AbstractRecordsHandler.h"


namespace db {
namespace fields {
namespace uuid_map {


using namespace std;


class BucketBlockRecord;
class BucketBlock:
    protected AbstractRecordsHandler {

    friend class BucketBlockTests;

public:
    explicit BucketBlock();
    explicit BucketBlock(
        byte *data);
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
    const RecordNumber recordsCount() const;
    const pair<shared_ptr<byte>, uint32_t> serializeToBytes() const;

protected:
    BucketBlockRecord *mRecords;
    RecordNumber mRecordsCount;
    bool mHasBeenModified;

protected:
    BucketBlockRecord* createRecord(
        const NodeUUID &uuid);
    void dropRecord(BucketBlockRecord *record);
};

} // namespace uuid_map
} // namespace fields
} // namespace db

#endif //GEO_NETWORK_CLIENT_BUCKETBLOCKDESCRIPTOR_H

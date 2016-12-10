#ifndef GEO_NETWORK_CLIENT_UUIDCOLUMN_H
#define GEO_NETWORK_CLIENT_UUIDCOLUMN_H


#include "../common/AbstractFileDescriptorHandler.h"
#include "../common/RecordNumbersIndex.h"
#include "../../../common/NodeUUID.h"
#include "internal/UUIDMapStorage.h"


namespace db {
namespace fields {
namespace uuid_map {


using namespace std;


class UUIDMapColumn {
public:
    typedef RecordNumbersIndex::RecordNumber RecordNumber;
    typedef RecordNumbersIndex::RecordsCount RecordsCount;

public:
    UUIDMapColumn(
        const char *filename,
        const char *path);

    void set(
        const RecordNumber recN,
        const NodeUUID &uuid);

    void remove(
        const RecordNumber recN,
        const NodeUUID &uuid);

    bool contains(
        const NodeUUID &uuid) const;

    inline const pair<RecordNumber*, RecordsCount>
        recordNumbersAssignedToUUID(
            const NodeUUID &uuid) const;

protected:
    RecordNumbersIndex *mRecordNumbersIndexHandler;
    UUIDMapStorage *mUUIDMapStorage;
};


} // namespace uuid_map
} // namespace fields
} // namespace db



#endif //GEO_NETWORK_CLIENT_UUIDCOLUMN_H

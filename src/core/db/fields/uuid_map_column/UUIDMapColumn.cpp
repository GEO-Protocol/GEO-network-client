#include "UUIDMapColumn.h"


namespace db {
namespace fields {
namespace uuid_map {


UUIDMapColumn::UUIDMapColumn(const char *filename,
                             const char *path) {


}


bool UUIDMapColumn::contains(const NodeUUID &uuid) const {


    return false;
}

const pair<UUIDMapColumn::RecordNumber *, UUIDMapColumn::RecordsCount>
UUIDMapColumn::recordNumbersAssignedToUUID(const NodeUUID &uuid) const {
    return mUUIDMapStorage->recordNumbersAssignedToUUID(uuid);
}


} // namespace uuid_map
} // namespace fields
} // namespace db



#ifndef GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H
#define GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H


#include "transactions/OperationsLog.h"
#include "transactions/Transaction.h"

#include "../../../db/fields/uuid_column/UUIDColumn.h"
#include "../../../db/fields/tl_direction_column/TrustLineDirectionColumn.h"

#include <boost/filesystem.hpp>


namespace io {
namespace routing_tables {


namespace fs = boost::filesystem;
using namespace db::fields;


class Transaction;
class AbstractRoutingTable:
    protected AbstractRecordsHandler {
    friend class AbstractRoutingTableTests;
    friend class Transaction;


public:
    AbstractRoutingTable(
        const fs::path &path,
        const uint8_t pow2bucketsCountIndex);

    ~AbstractRoutingTable();

    Transaction* beginTransaction() const;

protected:
    fs::path mF1Path;
    fs::path mF2Path;
    fs::path mDirColumnPath;

    UUIDColumn *mF1Column;
    UUIDColumn *mF2Column;
    TrustLineDirectionColumn *mDirColumn;
    OperationsLog *mOperationsLog;

protected:
    void set(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirection direction,
        const bool commit = true);

    void remove(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const bool commit = true);

    void commitOperations();
    void rollBackOperations();
    void rollbackSetOperation(
        SetOperation::ConstShared operation);

    const RecordNumber intersectingRecordNumber(
        const NodeUUID &u1,
        const NodeUUID &u2) const;

    void executeOperation(
        const Operation::Shared operation);
};


} // namespace routing tables
} // namespace io

#endif //GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H

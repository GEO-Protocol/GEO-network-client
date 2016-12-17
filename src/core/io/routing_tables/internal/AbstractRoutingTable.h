#ifndef GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H
#define GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H

#include "TransactionsHandler.h"

#include "../../../db/fields/uuid_map_column/UUIDMapColumn.h"
#include "../../../db/fields/tl_direction_column/TrustLineDirectionColumn.h"

#include <boost/filesystem.hpp>


namespace io {
namespace routing_tables {


namespace fs = boost::filesystem;
using namespace db::fields;


class AbstractRoutingTable:
    protected AbstractRecordsHandler {

    friend class AbstractRoutingTableTests;

public:
    enum Level {
        Second,
        Third,
    };

public:
    AbstractRoutingTable(
        const char *path,
        const Level level);

    ~AbstractRoutingTable();

    void set(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirectionColumn::Direction direction);

    void remove(
        const NodeUUID &u1,
        const NodeUUID &u2);

    // ...
    // The rest methods would be implemented when specification would be clear
    // ...

protected:
    fs::path mF1Path;
    fs::path mF2Path;
    fs::path mDirColumnPath;

    UUIDMapColumn *mF1Column;
    UUIDMapColumn *mF2Column;
    TrustLineDirectionColumn *mDirColumn;

    TransactionsHandler *mTransactionHandler;

protected:
    const RecordNumber intersectingRecordNumber(
        const NodeUUID &u1,
        const NodeUUID &u2) const;

    void processTransaction(
        const BaseTransaction *transaction);

    void executeTransaction(
        const BaseTransaction *transaction);

    void rollbackTransaction(
        const BaseTransaction *transaction);




};


} // namespace routing tables
} // namespace io

#endif //GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H

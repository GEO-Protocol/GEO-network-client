#ifndef GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H
#define GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H

#include "TransactionsHandler.h"

#include "../../../db/fields/uuid_map_column/UUIDMapColumn.h"
#include "../../../db/fields/tl_direction_column/TrustLineDirectionColumn.h"



namespace io {
namespace routing_tables {


using namespace db::fields;


class AbstractRoutingTable {
public:

public:
    AbstractRoutingTable(
        const char *path);

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
    const UUIDMapColumn::RecordNumber intersectingRecordNumber(
        const NodeUUID &u1,
        const NodeUUID &u2) const;

    void processTransaction(
        const BaseTransaction *transaction);

    void executeTransaction(
        const BaseTransaction *transaction);

    void rollbackTransaction(
        const BaseTransaction *transaction);

protected:
    UUIDMapColumn *mF1Column;
    UUIDMapColumn *mF2Column;
    TrustLineDirectionColumn *mDirColumn;

    TransactionsHandler *mTransactionHandler;
};


} // namespace routing tables
} // namespace io

#endif //GEO_NETWORK_CLIENT_ABSTRACTROUTINGTABLE_H

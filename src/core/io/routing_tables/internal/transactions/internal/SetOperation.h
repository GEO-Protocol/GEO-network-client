#ifndef GEO_NETWORK_CLIENT_SETTRANSACTION_H
#define GEO_NETWORK_CLIENT_SETTRANSACTION_H


#include "Operation.h"
#include "RollbackSetOperation.h"
#include "../../../../../common/NodeUUID.h"
#include "../../../../../common/exceptions/MemoryError.h"


namespace io {
namespace routing_tables {


class SetOperation :
    public Operation {

public:
    typedef shared_ptr<SetOperation> Shared;

    // Size of this instance in serialized format.
    static const constexpr size_t kSerializedSize =
        + sizeof(Operation::SerializedOperationType)
        + NodeUUID::kUUIDLength
        + NodeUUID::kUUIDLength
        + sizeof(Operation::SerializedTrustLineDirectionType)
        + sizeof(RecordNumber);

public:
    explicit SetOperation(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirection direction,
        const RecordNumber recN);

    explicit SetOperation(
        const byte *data);

    const NodeUUID& u1() const;
    const NodeUUID& u2() const;
    const TrustLineDirection direction() const;
    const RecordNumber recordNumber() const;

    virtual const pair<shared_ptr<byte>, size_t> serialize() const;
    const RollbackSetOperation::Shared rollbackOperation() const;

protected:
    NodeUUID mU1;
    NodeUUID mU2;
    TrustLineDirection mDirection;
    RecordNumber mRecN;
};


}
}

#endif //GEO_NETWORK_CLIENT_SETTRANSACTION_H

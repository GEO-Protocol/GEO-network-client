#ifndef GEO_NETWORK_CLIENT_DIRECTIONUPDATETRANSACTION_H
#define GEO_NETWORK_CLIENT_DIRECTIONUPDATETRANSACTION_H


#include "Operation.h"
#include "RollbackDirectionUpdateOperation.h"
#include "../../../../../common/exceptions/MemoryError.h"


namespace io {
namespace routing_tables {


class DirectionUpdateOperation:
    public Operation {

public:
    typedef shared_ptr<DirectionUpdateOperation> Shared;

    // Size of this instance in serialized format.
    static const constexpr size_t kSerializedSize =
        + sizeof(Operation::SerializedOperationType)
        + sizeof(RecordNumber)
        + sizeof(Operation::SerializedTrustLineDirectionType)
        + sizeof(Operation::SerializedTrustLineDirectionType);

public:
    explicit DirectionUpdateOperation(
        const RecordNumber recN,
        const TrustLineDirection direction,
        const TrustLineDirection directionBackup);

    explicit DirectionUpdateOperation(
        const byte *data);

    const RecordNumber recordNumber() const;
    const TrustLineDirection direction() const;
    const TrustLineDirection directionBackup() const;

    const pair<shared_ptr<byte>, size_t> serialize() const;
    const RollbackDirectionUpdateOperation::Shared rollbackOperation() const;

protected:
    RecordNumber mRecN;
    TrustLineDirection mDirection;
    TrustLineDirection mDirectionBackup;
};


}
}

#endif //GEO_NETWORK_CLIENT_DIRECTIONUPDATETRANSACTION_H

#ifndef GEO_NETWORK_CLIENT_REMOVETRANSACTION_H
#define GEO_NETWORK_CLIENT_REMOVETRANSACTION_H


#include "SetOperation.h"
#include "RollbackRemoveOperation.h"


namespace io {
namespace routing_tables {

/*!
 * RemoveOperation inherits from SetOperation
 * to be able to revert remove operation:
 * remove operation may be processed with u1 and u2 fields only,
 * but direction is stored to be able to revert the operation
 * via the InsertTransaction.
 */
class RemoveOperation:
    public SetOperation {

public:
    typedef shared_ptr<RemoveOperation> Shared;
    typedef shared_ptr<RollbackRemoveOperation> RollbackShared;

public:
    using SetOperation::SetOperation;
    explicit RemoveOperation(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirection direction);

    virtual const pair<shared_ptr<byte>, size_t> serialize() const;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
    const RemoveOperation::RollbackShared rollbackOperation() const;
#pragma clang diagnostic pop
};


}
}



#endif //GEO_NETWORK_CLIENT_REMOVETRANSACTION_H

#include "RollbackRemoveOperation.h"
#include "SetOperation.h"


namespace io {
namespace routing_tables {


RollbackRemoveOperation::RollbackRemoveOperation(
    const NodeUUID &u1,
    const NodeUUID &u2,
    TrustLineDirection direction):
    Operation(RollbackRemove),
    mU1(u1),
    mU2(u2),
    mDir(direction){}

const TrustLineDirection RollbackRemoveOperation::direction() const {
    return mDir;
}

const NodeUUID &RollbackRemoveOperation::u1() const {
    return mU1;
}

const NodeUUID &RollbackRemoveOperation::u2() const {
    return mU2;
}

const pair<shared_ptr<byte>, size_t> RollbackRemoveOperation::serialize() const {
    throw RuntimeError(
        "RollbackRemoveOperation::serialize: "
            "rollback operation can't shouldn't be serialized.");
}


}
}

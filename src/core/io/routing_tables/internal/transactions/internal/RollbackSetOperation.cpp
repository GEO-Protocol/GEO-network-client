#include "RollbackSetOperation.h"


namespace io {
namespace routing_tables {


RollbackSetOperation::RollbackSetOperation(
    const NodeUUID &u1,
    const NodeUUID &u2):
    Operation(RollbackSet),
    mU1(u1),
    mU2(u2) {}

const pair<shared_ptr<byte>, size_t> RollbackSetOperation::serialize() const {
    throw RuntimeError(
        "RollbackSetOperation::serialize: "
            "rollback operation can't shouldn't be serialized.");
}

const NodeUUID &RollbackSetOperation::u1() const {
    return mU1;
}

const NodeUUID &RollbackSetOperation::u2() const {
    return mU2;
}


}
}



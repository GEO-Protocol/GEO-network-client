#include "RemoveOperation.h"


namespace io {
namespace routing_tables {


RemoveOperation::RemoveOperation(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirection direction,
    const RecordNumber recN):
    SetOperation(u1, u2, direction, recN) {

    mType = Remove;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
const RollbackRemoveOperation::Shared RemoveOperation::rollbackOperation() const {
    try {
        return RollbackRemoveOperation::Shared(
            new RollbackRemoveOperation(mU1, mU2, mDirection));

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
    } catch (std::bad_alloc &) {
        throw MemoryError(
            "RemoveOperation::RollbackShared: bad allocation.");
#pragma clang diagnostic pop
    }
}
#pragma clang diagnostic pop

const pair<shared_ptr<byte>, size_t> RemoveOperation::serialize() const {
    return SetOperation::serialize();
}


}
}

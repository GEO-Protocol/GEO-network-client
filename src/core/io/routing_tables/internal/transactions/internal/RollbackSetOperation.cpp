#include "RollbackSetOperation.h"


namespace io {
namespace routing_tables {


RollbackSetOperation::RollbackSetOperation(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const RecordNumber recN):
    Operation(RollbackSet),
    mU1(u1),
    mU2(u2),
    mRecN(recN){}

const pair<shared_ptr<byte>, size_t> RollbackSetOperation::serialize() const {
    throw RuntimeError(
        "RollbackSetOperation::serialize: "
            "rollback operation shouldn't be serialized.");
}

const NodeUUID &RollbackSetOperation::u1() const {
    return mU1;
}

const NodeUUID &RollbackSetOperation::u2() const {
    return mU2;
}

const AbstractRecordsHandler::RecordNumber RollbackSetOperation::recordNumber() const {
    return mRecN;
}


}
}



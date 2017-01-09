#include "RollbackDirectionUpdateOperation.h"


namespace io {
namespace routing_tables {


RollbackDirectionUpdateOperation::RollbackDirectionUpdateOperation(
    const RecordNumber recN,
    TrustLineDirection direction):
    Operation(RollbackUpdate),
    mRecN(recN),
    mDir(direction){}

const AbstractRecordsHandler::RecordNumber RollbackDirectionUpdateOperation::recordNumber() const {
    return mRecN;
}

const TrustLineDirection RollbackDirectionUpdateOperation::direction() const {
    return mDir;
}

const pair<shared_ptr<byte>, size_t> RollbackDirectionUpdateOperation::serialize() const {
    throw RuntimeError(
        "RollbackDirectionUpdateOperation::serialize: "
            "rollback operation can't be serialized.");
}


}
}



#include "AbstractRoutingTable.h"

namespace io {
namespace routing_tables {


AbstractRoutingTable::AbstractRoutingTable(
    const fs::path &path,
    const uint8_t pow2bucketsCountIndex):

    // Each column would be located into its sub-directory.
    mF1Path(path / fs::path("f1/")),
    mF2Path(path / fs::path("f2/")),
    mDirColumnPath(path / fs::path("dir/")) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(pow2bucketsCountIndex > 0 && pow2bucketsCountIndex < 16);
#endif

    try {
        mF1Column = new UUIDColumn(mF1Path, pow2bucketsCountIndex);
        mF2Column = new UUIDColumn(mF2Path, pow2bucketsCountIndex);
        mDirColumn = new TrustLineDirectionColumn(mDirColumnPath);
        mOperationsLog = new OperationsLog(path);

    } catch (std::bad_alloc &e) {
        throw MemoryError(
            "AbstractRoutingTable::AbstractRoutingTable: "
                "can't allocate enough memory internal components.");
    }

    // In case if transactions log contains operations -
    // all of them should be rolled back;
    rollBackOperations();
}

AbstractRoutingTable::~AbstractRoutingTable() {
    delete mF1Column;
    delete mF2Column;
    delete mDirColumn;
    delete mOperationsLog;
}

Transaction *AbstractRoutingTable::beginTransaction() const {
    if (mOperationsLog->transactionMayBeStarted()) {
        return new Transaction(this);

    } else {
        throw ConflictError(
            "AbstractRoutingTable::beginTransaction: "
                "other transaction is in progress.");
    }
}

void AbstractRoutingTable::set(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirection direction,
    const bool commit) {

    // (u1, u2) is primary key.
    // In case when this PK is already present in the table -
    // only direction should be updated.
    // Otherwise - new record should be inserted.
    try {
        auto recN = intersectingRecordNumber(u1, u2);
        auto directionBackup = mDirColumn->direction(recN);
        executeOperation(mOperationsLog->initDirectionUpdateOperation(recN, direction, directionBackup));

    } catch (IndexError &){
        // New record should be inserted
        executeOperation(mOperationsLog->initSetOperation(u1, u2, direction));
    }

    if (commit) {
        commitOperations();
    }
}

void AbstractRoutingTable::remove(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const bool commit) {

    // (u1, u2) is primary key.
    // In case when this PK is already present in the table -
    // only direction should be updated.
    // Otherwise - new record should be inserted.
    try {
        auto recN = intersectingRecordNumber(u1, u2);

        // backup direction to be able to roll back removing,
        // in case when transaction will crash.
        auto direction = mDirColumn->direction(recN);

//        processTransaction(
//            mOperationsLog->initRemoveOperation(u1, u2, direction));

        if (commit) {
            commitOperations();
        }

    } catch (IndexError &){
        throw NotFoundError(
            "AbstractRoutingTable::remove: "
                "record with pk=(u1, u2) doesn't exists.");
    }
}

void AbstractRoutingTable::commitOperations() {

}

void AbstractRoutingTable::rollBackOperations() {
    auto uncompletedOperations = mOperationsLog->uncompletedOperations();
    if (uncompletedOperations->size() > 0) {
        for (auto &op: *uncompletedOperations) {
            switch (op->type()) {
                case Operation::Set: {
                    rollbackSetOperation(
                        dynamic_pointer_cast<SetOperation>(op));
                    continue;
                }

                default: {
                    throw RuntimeError(
                        "AbstractRoutingTable::rollBackOperations: "
                            "unexpected operation occurred.");
                }
            }
        }
    }
}

const AbstractRecordsHandler::RecordNumber AbstractRoutingTable::intersectingRecordNumber(
    const NodeUUID &u1,
    const NodeUUID &u2) const {

    auto f1RecordNumbers = mF1Column->recordNumbersAssignedToUUID(u1);
    if (f1RecordNumbers.second == 0) {
        throw IndexError(
            "AbstractRoutingTable::intersectingRecordNumber: "
                "first field doesn't contains any record numbers.");
    }

    auto f2RecordNumbers = mF2Column->recordNumbersAssignedToUUID(u2);
    if (f2RecordNumbers.second == 0) {
        throw IndexError(
            "AbstractRoutingTable::intersectingRecordNumber: "
                "second field doesn't contains any record numbers.");
    }

    // Intersecting of received record numbers
    auto f1Offset = f1RecordNumbers.first;
    auto f2Offset = f2RecordNumbers.first;

    for (size_t f1RecordIndex=0; f1RecordIndex<f1RecordNumbers.second; ++f1RecordIndex) {
        for (size_t f2RecordIndex=0; f2RecordIndex<f1RecordNumbers.second; ++f2RecordIndex) {
            if (f1Offset[f1RecordIndex] == f2Offset[f2RecordIndex])
                return f1Offset[f1RecordIndex];
        }
    }

    throw IndexError(
        "AbstractRoutingTable::intersectingRecordNumber: "
            "there is intersecting record number between first field and second field.");
}

void AbstractRoutingTable::executeOperation(
    const Operation::Shared operation) {

    if (operation->type() == Operation::Set) {
        const SetOperation *op = dynamic_cast<SetOperation*>(operation.get());

        mF1Column->set(op->u1(), op->recordNumber());
        mF2Column->set(op->u2(), op->recordNumber());
        mDirColumn->set(op->recordNumber(), op->direction());
        return;

    } else {
        throw Exception(
            "AbstractRoutingTable::executeTransaction: "
                "unexpected transaction type occurred.");
    }
}

void AbstractRoutingTable::rollbackSetOperation(
    SetOperation::Shared operation) {

}

} // namespace routing tables
} // namespace io
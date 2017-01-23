#include "AbstractRoutingTable.h"

namespace io {
namespace routing_tables {


AbstractRoutingTable::AbstractRoutingTable(
    const fs::path &path,
    const uint8_t pow2bucketsCountIndex):

    // Each column would be located into its sub-directory.
    mF1Path(path / fs::path("u1/")),
    mF2Path(path / fs::path("u2/")),
    mDirColumnPath(path / fs::path("directions/")) {

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

shared_ptr<Transaction> AbstractRoutingTable::beginTransaction() const {
    if (mOperationsLog->transactionMayBeStarted()) {
        return shared_ptr<Transaction>(new Transaction(this));

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
        executeOperation(mOperationsLog->initRemoveOperation(u1, u2, direction, recN));

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
    mF1Column->commitCachedBlocks();
    mF2Column->commitCachedBlocks();
    mDirColumn->commit();

    mOperationsLog->commit();
}

/*
 * Reverts all operations that are currently in the operations log.
 *
 *
 * Throws RuntimeError in case when uncompleted operations can't be loaded, or correctly processed.
 */
void AbstractRoutingTable::rollBackOperations() {
    try {
        auto uncompletedOperations = mOperationsLog->uncompletedOperations();
        if (uncompletedOperations->size() > 0) {
            for (auto &op: *uncompletedOperations) {
                switch (op->type()) {
                    case Operation::Set: {
                        executeOperation(static_pointer_cast<const SetOperation>(op)->rollbackOperation());
                        break;
                    }
                    case Operation::Remove: {
                        executeOperation(static_pointer_cast<const RemoveOperation>(op)->rollbackOperation());
                        break;
                    }
                    case Operation::Update: {
                        executeOperation(static_pointer_cast<const DirectionUpdateOperation>(op)->rollbackOperation());
                        break;
                    }
                    default: {
                        throw RuntimeError(
                            "AbstractRoutingTable::rollBackOperations: invalid operation type received.");
                    }
                }
            }

            commitOperations();
        }

    } catch (Exception &e) {
        throw RuntimeError(
            std::string("AbstractRoutingTable::rollBackOperations: unexpected operation occurred. Details are: ")
            + e.message());
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
    Operation::ConstShared operation) {

    if (operation->type() == Operation::Set) {
        auto op = dynamic_pointer_cast<const SetOperation>(operation);
        mF1Column->set(op->u1(), op->recordNumber(), false);
        mF2Column->set(op->u2(), op->recordNumber(), false);
        mDirColumn->set(op->recordNumber(), op->direction(), false);
        return;


    } else if (operation->type() == Operation::RollbackSet) {
        auto op = dynamic_pointer_cast<const RollbackSetOperation>(operation);
        mF1Column->remove(op->u1(), op->recordNumber(), false);
        mF2Column->remove(op->u2(), op->recordNumber(), false);
        mDirColumn->remove(op->recordNumber(), false);
        return;


    } else if (operation->type() == Operation::Update) {
        auto op = dynamic_pointer_cast<const DirectionUpdateOperation>(operation);
        mDirColumn->set(op->recordNumber(), op->direction(), false);


    } else if (operation->type() == Operation::RollbackUpdate) {
        auto op = dynamic_pointer_cast<const RollbackDirectionUpdateOperation>(operation);
        mDirColumn->set(op->recordNumber(), op->direction(), false);


    } else if (operation->type() == Operation::Remove) {
        auto op = dynamic_pointer_cast<const RemoveOperation>(operation);
        mF1Column->remove(op->u1(), op->recordNumber(), false);
        mF2Column->remove(op->u2(), op->recordNumber(), false);
        mDirColumn->remove(op->recordNumber(), false);

    } else if (operation->type() == Operation::RollbackRemove) {
        auto op = dynamic_pointer_cast<const RollbackRemoveOperation>(operation);
        auto nextRecordNumber = mOperationsLog->nextRecordNumber();
        mF1Column->set(op->u1(), nextRecordNumber, false);
        mF2Column->set(op->u2(), nextRecordNumber, false);
        mDirColumn->set(nextRecordNumber, op->direction(), false);


    } else {
        throw RuntimeError(
            "AbstractRoutingTable::executeTransaction: "
                "unexpected transaction transactionType occurred.");
    }
}


} // namespace routing tables
} // namespace io
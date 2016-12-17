#include "AbstractRoutingTable.h"

namespace io {
namespace routing_tables {


AbstractRoutingTable::AbstractRoutingTable(
    const char *path,
    const Level level):

    // Each column would be located into its sub-directory.
    mF1Path(string(path) + "/f1/"),
    mF2Path(string(path) + "/f2/"),
    mDirColumnPath(string(path) + "/dir/") {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(path != nullptr);
#endif

    uint8_t pow2bucketsIndex;
    switch (level) {
        case Second:
            pow2bucketsIndex = 9; // 2**9 = 512 buckets on the disk.
            break;

        case Third:
            pow2bucketsIndex = 13; // 2**13 = 8192 buckets on the disk.
            break;

        default:
            throw ValueError(
                "AbstractRoutingTable::AbstractRoutingTable: "
                    "invalid \"level\" received.");
    }

    mF1Column = new UUIDMapColumn(mF1Path.c_str(), pow2bucketsIndex);
    mF2Column = new UUIDMapColumn(mF2Path.c_str(), pow2bucketsIndex);
    mDirColumn = new TrustLineDirectionColumn("dir.col", path);

    mTransactionHandler = new TransactionsHandler("tr.dat", path);
}

AbstractRoutingTable::~AbstractRoutingTable() {
    delete mF1Column;
    delete mF2Column;
    delete mDirColumn;
    delete mTransactionHandler;
}

void AbstractRoutingTable::set(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirectionColumn::Direction direction) {

    // (u1, u2) is primary key.
    // In case when this PK is already present in the table -
    // only direction should be updated.
    // Otherwise - new record should be inserted.
    try {
        auto recN = intersectingRecordNumber(u1, u2);
        processTransaction(
            mTransactionHandler->beginDirectionUpdateTransaction(recN, direction));

    } catch (IndexError &){
        // New record should be inserted
        processTransaction(
            mTransactionHandler->beginInsertTransaction(u1, u2, direction));
    }
}

void AbstractRoutingTable::remove(
    const NodeUUID &u1,
    const NodeUUID &u2) {

    // (u1, u2) is primary key.
    // In case when this PK is already present in the table -
    // only direction should be updated.
    // Otherwise - new record should be inserted.
    try {
        auto recN = intersectingRecordNumber(u1, u2);

        // backup direction to be able to roll back removing,
        // in case when transaction will crash.
        auto direction = mDirColumn->direction(recN);

        processTransaction(
            mTransactionHandler->beginRemoveTransaction(u1, u2, direction, recN));

    } catch (IndexError &){
        throw NotFoundError(
            "AbstractRoutingTable::remove: "
                "record with pk=(u1, u2) doesn't exists.");
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

void AbstractRoutingTable::processTransaction(
    const BaseTransaction *transaction) {

    try {
        executeTransaction(transaction);
        mTransactionHandler->commitLastTransaction();
        delete transaction;

    } catch (Exception &e) {
        // todo: process errors

        rollbackTransaction(transaction);
        mTransactionHandler->removeLastTransaction();
        delete transaction;

        throw e;
    }
}

void AbstractRoutingTable::executeTransaction(
    const BaseTransaction *transaction) {

    switch (transaction->type) {
        case BaseTransaction::RecordInserting: {
            const InsertTransaction *insertTransaction =
                dynamic_cast<const InsertTransaction*>(transaction);

            mF1Column->set(insertTransaction->u1, insertTransaction->recordNumber);
            mF2Column->set(insertTransaction->u2, insertTransaction->recordNumber);
            mDirColumn->set(insertTransaction->recordNumber, insertTransaction->direction);
        }

        case BaseTransaction::RecordRemoving: {
            const RemoveTransaction *removeTransaction =
                dynamic_cast<const RemoveTransaction*>(transaction);

            mF1Column->remove(removeTransaction->u1, removeTransaction->recordNumber);
            mF2Column->remove(removeTransaction->u2, removeTransaction->recordNumber);
            mDirColumn->remove(removeTransaction->recordNumber);
        }

        case BaseTransaction::DirectionUpdating: {
            const DirectionUpdateTransaction *updateTransaction =
                dynamic_cast<const DirectionUpdateTransaction*>(transaction);

            mDirColumn->set(updateTransaction->recordNumber, updateTransaction->direction);
        }

        default: {
            throw Exception(
                "AbstractRoutingTable::executeTransaction: "
                    "unexpected transaction type occurred.");
        }
    }
}

void AbstractRoutingTable::rollbackTransaction(
    const BaseTransaction *transaction) {

    switch (transaction->type) {
        case BaseTransaction::RecordInserting: {
            // Reverting of the insert transaction is
            // removing of partially inserted record.

            const InsertTransaction *insertTransaction =
                dynamic_cast<const InsertTransaction*>(transaction);

            try {
                mF1Column->remove(insertTransaction->u1, insertTransaction->recordNumber);
            } catch (NotFoundError &e) {}

            try {
                mF2Column->remove(insertTransaction->u2, insertTransaction->recordNumber);
            } catch (NotFoundError &e) {}

            try {
                mDirColumn->remove(insertTransaction->recordNumber);
            } catch (NotFoundError &e) {}
        }

        case BaseTransaction::RecordRemoving: {
            // Reverting of removing transaction is
            // inserting record back.

            const RemoveTransaction *removeTransaction =
                dynamic_cast<const RemoveTransaction*>(transaction);

            try {
                mF1Column->set(removeTransaction->u1, removeTransaction->recordNumber);
            } catch (ConflictError &e) {}

            try {
                mF2Column->set(removeTransaction->u2, removeTransaction->recordNumber);
            } catch (ConflictError &e) {}

            try {
                mDirColumn->set(removeTransaction->recordNumber, removeTransaction->direction);
            } catch (ConflictError &e) {}

        }

        case BaseTransaction::DirectionUpdating: {
            const DirectionUpdateTransaction *updateTransaction =
                dynamic_cast<const DirectionUpdateTransaction*>(transaction);

            mDirColumn->set(updateTransaction->recordNumber, updateTransaction->direction);
        }

        default: {
            throw Exception(
                "AbstractRoutingTable::executeTransaction: "
                    "unexpected transaction type occurred.");
        }
    }
}


} // namespace routing tables
} // namespace io
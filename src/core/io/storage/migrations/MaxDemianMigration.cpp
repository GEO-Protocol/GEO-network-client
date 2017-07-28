#include "MaxDemianMigration.h"

MaxDemianMigration::MaxDemianMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mDataBase(dbConnection),
    mLog(logger)
{
    mMigrateNodeUUID = NodeUUID("9e2e8dff-a102-449a-92aa-f6be725be291");
    mNeighborUUIDCreditor = NodeUUID("1a3da803-f7bd-46a1-95d5-ccf8ba24d2bd");
    mNeighborUUIDDebtor = NodeUUID("547f80d2-1f34-46c4-8dde-528441cabece");

    mOperationAmount = TrustLineAmount(499);
}

void MaxDemianMigration::apply(IOTransaction::Shared ioTransaction) {
    auto trustLine_debor = getOutwornTrustLine(ioTransaction, mNeighborUUIDDebtor);
    auto trustLine_creditor = getOutwornTrustLine(ioTransaction, mNeighborUUIDCreditor);

    if (trustLine_debor and trustLine_creditor) {
        auto new_balance = trustLine_debor->balance() + mOperationAmount;
        trustLine_debor->setBalance(new_balance);
        ioTransaction->trustLineHandler()->saveTrustLine(trustLine_debor);

        new_balance = trustLine_creditor->balance() - mOperationAmount;
        trustLine_creditor->setBalance(new_balance);
        ioTransaction->trustLineHandler()->saveTrustLine(trustLine_creditor);
    } else {
        throw RuntimeError("Can not find TrustLines to migrate");
    }
}

std::shared_ptr <TrustLine> MaxDemianMigration::getOutwornTrustLine(
    IOTransaction::Shared ioTransaction,
    NodeUUID &nodeUUID) {
    sqlite3_stmt *stmt;
    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " +
                   ioTransaction->trustLineHandler()->tableName() + " where contractor = ?";


    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "SolomonHistoryMigration::getOutwornTrustLine: "
                "Can't select trust lines count; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_bind_blob(stmt, 1, nodeUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError(
            "TrustLineHandler::insert or replace: "
                "Bad binding of Contractor; sqlite error: " + to_string(rc));
    }
    TrustLine::Shared result;

    // For each trust line in the table, deserialize it with old method and serialize it back with new one.
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID contractor(static_cast<const byte *>(sqlite3_column_blob(stmt, 0)));

        auto incomingAmountBytes = static_cast<const byte*>(sqlite3_column_blob(stmt, 1));
        vector<byte> incomingAmountBufferBytes(
            incomingAmountBytes,
            incomingAmountBytes + kTrustLineAmountBytesCount);
        TrustLineAmount incomingAmount = bytesToTrustLineAmount(incomingAmountBufferBytes);

        auto outgoingAmountBytes = static_cast<const byte*>(sqlite3_column_blob(stmt, 2));
        vector<byte> outgoingAmountBufferBytes(
            outgoingAmountBytes,
            outgoingAmountBytes + kTrustLineAmountBytesCount);
        TrustLineAmount outgoingAmount = bytesToTrustLineAmount(outgoingAmountBufferBytes);

        auto balanceBytes = static_cast<const byte*>(sqlite3_column_blob(stmt, 3));
        vector<byte> balanceBufferBytes(
            balanceBytes,
            balanceBytes + kTrustLineBalanceSerializeBytesCount);
        TrustLineBalance balance = bytesToTrustLineBalance(balanceBufferBytes);

        try {
            return make_shared<TrustLine>(
                contractor,
                incomingAmount,
                outgoingAmount,
                balance);

        } catch (...) {
            throw RuntimeError(
                "MaxDemianMigration::getOutwornTrustLine: "
                    "Unable to create TL.");
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

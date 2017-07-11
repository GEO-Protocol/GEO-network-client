#include "SolomonHistoryMigration.h"

SolomonHistoryMigration::SolomonHistoryMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mLog(logger),
    mDataBase(dbConnection)
{
    mMigrateNodeUUID = NodeUUID("2136a78d-3cb0-488d-b1a6-e039c12689d0");
    mCoordinatorUUID = NodeUUID("ba684d43-a66e-4840-9b29-679dcd679f23");
    mNeighborUUID = NodeUUID("15ce1178-ba9d-4172-845d-48d005dae106");

    mOperationUUID = TransactionUUID(string("a4bebc20-8c87-448c-9a92-c4f86e28d2e2"));
    mPreviousOperationUUID = TransactionUUID(string("11809a84-8301-4c9f-84c1-bc7820968c47"));

    mOperationAmount = TrustLineBalance(2282);
}

void SolomonHistoryMigration::apply(IOTransaction::Shared ioTransaction) {

    applyTrustLineMigration(ioTransaction);
    applyTrustLineHistoryMigration(ioTransaction);
}

void SolomonHistoryMigration::applyTrustLineMigration(IOTransaction::Shared ioTransaction) {

    return;
}

void SolomonHistoryMigration::applyTrustLineHistoryMigration(IOTransaction::Shared ioTransaction) {
    auto trustline = getOutwornTrustLine(ioTransaction);
    auto new_balance = trustline->balance() + mOperationAmount;
    trustline->setBalance(new_balance);
    ioTransaction->trustLineHandler()->saveTrustLine(trustline);
}


TrustLine::Shared SolomonHistoryMigration::getOutwornTrustLine(
    IOTransaction::Shared ioTransaction)
{
    sqlite3_stmt *stmt;
    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " +
                   ioTransaction->trustLineHandler()->tableName() + " where contractor = ?";


    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "SolomonHistoryMigration::getOutwornTrustLine: "
                "Can't select trust lines count; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_bind_blob(stmt, 1, mNeighborUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert or replace: "
                          "Bad binding of Contractor; sqlite error: " + rc);
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
                "SolomonHistoryMigration::getOutwornTrustLine: "
                    "Unable to create TL.");
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

#include "MaxDemianMigration.h"

MaxDemianMigration::MaxDemianMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mDataBase(dbConnection),
    mLog(logger)
{
    mNeighborUUIDFirst = NodeUUID("1a3da803-f7bd-46a1-95d5-ccf8ba24d2bd");
    mNeighborUUIDSecond = NodeUUID("547f80d2-1f34-46c4-8dde-528441cabece");
    mNeighborUUIDThird = NodeUUID("0bc6a4fa-154a-4cbd-9409-79d0a1c3e2f1");

    mOperationAmountFirstAndSecond = TrustLineAmount(499);
    mOperationAmountThird = TrustLineAmount(36456);
}

void MaxDemianMigration::apply(IOTransaction::Shared ioTransaction) {

    auto trustLine_first = getOutwornTrustLine(ioTransaction, mNeighborUUIDFirst);
    auto trustLine_second = getOutwornTrustLine(ioTransaction, mNeighborUUIDSecond);
    auto trustLine_third = getOutwornTrustLine(ioTransaction, mNeighborUUIDThird);

    if (trustLine_second and trustLine_first and trustLine_third) {
        auto new_balance = trustLine_second->balance() + mOperationAmountFirstAndSecond;
        trustLine_second->setBalance(new_balance);
        ioTransaction->trustLineHandler()->saveTrustLine(trustLine_second);

        new_balance = trustLine_first->balance() - mOperationAmountFirstAndSecond;
        trustLine_first->setBalance(new_balance);
        ioTransaction->trustLineHandler()->saveTrustLine(trustLine_first);


        new_balance = trustLine_third->balance() - mOperationAmountThird;
        trustLine_third->setBalance(new_balance);
        ioTransaction->trustLineHandler()->saveTrustLine(trustLine_third);

    } else {
        throw RuntimeError("Can not find TrustLines to migrate");
    }
}

std::shared_ptr <TrustLine> MaxDemianMigration::getOutwornTrustLine(
    IOTransaction::Shared ioTransaction,
    NodeUUID &nodeUUID) {
    sqlite3_stmt *stmt;
    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " +
                   ioTransaction->trustLineHandler()->tableName() + " WHERE contractor = ?";


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

    if (sqlite3_step(stmt) == SQLITE_ROW ) {
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

            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);

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
    } else {

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);

        throw NotFoundError(
            "MaxDemianMigration::getOutwornTrustLine: "
                "Unable to find TL.");
    }
}

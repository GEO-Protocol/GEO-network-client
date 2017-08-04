#include "DeleteSerializedTransactionsMigration.h"

DeleteSerializedTransactionsMigration::DeleteSerializedTransactionsMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mLog(logger),
    mDataBase(dbConnection)
{
    mTransactionUUID = TransactionUUID("3bb13828-63a8-43ba-b0be-0da5e6c4d7db");
}

void DeleteSerializedTransactionsMigration::apply(IOTransaction::Shared ioTransaction) {
    sqlite3_stmt *stmt;
    string query = "DELETE FROM " +
                   ioTransaction->transactionHandler()->tableName() + " WHERE transaction_uuid = ?";

    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "DeleteSerializedTransactionsMigration::apply: "
                "Can't delete transaction; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_bind_blob(stmt, 1, mTransactionUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError(
            "DeleteSerializedTransactionsMigration::insert or replace: "
                "Bad binding of transacrtion_uuid; sqlite error: " + to_string(rc));


    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("DeleteSerializedTransactionsMigration::deleteTransaction: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}


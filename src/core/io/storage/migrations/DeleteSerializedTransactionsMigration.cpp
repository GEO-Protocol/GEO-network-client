#include "DeleteSerializedTransactionsMigration.h"

DeleteSerializedTransactionsMigration::DeleteSerializedTransactionsMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mLog(logger),
    mDataBase(dbConnection)
{
}

void DeleteSerializedTransactionsMigration::apply(IOTransaction::Shared ioTransaction) {
    sqlite3_stmt *stmt;
    string query = "DELETE FROM " +
                   ioTransaction->transactionHandler()->tableName();

    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "DeleteSerializedTransactionsMigration::apply: "
                "Can't delete transaction; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("DeleteSerializedTransactionsMigration::deleteTransaction: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}


#include "UniqueIndexHistoryMigration.h"

UniqueIndexHistoryMigration::UniqueIndexHistoryMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mDataBase(dbConnection),
    mLog(logger)
{

}

void UniqueIndexHistoryMigration::apply(IOTransaction::Shared ioTransaction) {
    migrateHistory(ioTransaction);
    migrateHistoryAdditional(ioTransaction);
}

void UniqueIndexHistoryMigration::migrateHistory(IOTransaction::Shared ioTransaction)
{
    sqlite3_stmt *stmt;

    string query = "CREATE UNIQUE INDEX IF NOT EXISTS " + ioTransaction->historyStorage()->mainTableName()
            + "_operation_uuid_u_idx on " + ioTransaction->historyStorage()->mainTableName() + " (operation_uuid);";
    auto rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("UniqueIndexHistoryMigration::creating index for OperationUUID: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("UniqueIndexHistoryMigration::creating index for OperationUUID: "
                          "Run query; sqlite error: " + rc);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void UniqueIndexHistoryMigration::migrateHistoryAdditional(IOTransaction::Shared ioTransaction)
{
    sqlite3_stmt *stmt;

    string query = "CREATE UNIQUE INDEX IF NOT EXISTS " + ioTransaction->historyStorage()->additionalTableName()
                   + "_operation_uuid_u_idx on " + ioTransaction->historyStorage()->additionalTableName() + " (operation_uuid);";
    auto rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("UniqueIndexHistoryMigration::creating index for OperationUUID: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("UniqueIndexHistoryMigration::creating index for OperationUUID: "
                          "Run query; sqlite error: " + rc);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

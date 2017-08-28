#include "CommandUUIDMigration.h"


CommandUUIDMigration::CommandUUIDMigration(
        sqlite3 *dbConnection,
        Logger &logger):
        mDataBase(dbConnection),
        mLog(logger)
{

}

void CommandUUIDMigration::apply(IOTransaction::Shared ioTransaction) {
    try {
        sqlite3_stmt *stmt;

        string query = "CREATE UNIQUE INDEX IF NOT EXISTS " + ioTransaction->historyStorage()->mainTableName()
                       + "_operation_uuid_u_idx on " + ioTransaction->historyStorage()->mainTableName() + " (operation_uuid);";
        auto rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            throw IOError("UniqueIndexHistoryMigration::creating index for OperationUUID: "
                                  "Bad query; sqlite error: " + to_string(rc));
        }
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
        } else {
            throw IOError("UniqueIndexHistoryMigration::creating index for OperationUUID: "
                                  "Run query; sqlite error: " + to_string(rc));
        }
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    } catch(IOError& e){
        throw(e);
    }
}


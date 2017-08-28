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
        string query = "SELECT command_uuid FROM " + ioTransaction->historyStorage()->mainTableName() + " LIMIT 1";
        auto rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            query = "alter table " + ioTransaction->historyStorage()->mainTableName()
                    + " add column command_uuid BLOB";
            rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
            if (rc != SQLITE_OK) {
                throw IOError("CommandUUIDMigration::add new column command_uuid"
                                      "Bad query; sqlite error: ");
            }
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) {
            } else {
                throw IOError("CommandUUIDMigration::add new column command_uuid: "
                                      "Run query; sqlite error: " + to_string(rc));
            }

            query = "CREATE INDEX IF NOT EXISTS " + ioTransaction->historyStorage()->mainTableName()
                    + "_command_uuid_idx on " + ioTransaction->historyStorage()->mainTableName() + "(command_uuid);";
            rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
            if (rc != SQLITE_OK) {
                throw IOError("HistoryStorage::main creating index for command_uuid: "
                                      "Bad query; sqlite error: " + to_string(rc));
            }
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) {
            } else {
                throw IOError("HistoryStorage::main creating index for command_uuid: "
                                      "Run query; sqlite error: " + to_string(rc));
            }

            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
        }

    } catch(IOError& e){
        throw(e);
    }
}


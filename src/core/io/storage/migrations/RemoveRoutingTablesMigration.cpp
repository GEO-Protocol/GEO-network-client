#include "RemoveRoutingTablesMigration.h"

RemoveRoutingTablesMigration::RemoveRoutingTablesMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mDataBase(dbConnection),
    mLog(logger)
{}

void RemoveRoutingTablesMigration::apply(
    IOTransaction::Shared ioTransaction)
{
    try {
        sqlite3_stmt *stmt;
        string query = "DROP TABLE IF EXISTS rt2";
        auto rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);

        if (rc != SQLITE_OK) {
            throw IOError("RemoveRoutingTablesMigration::drop rt2 "
                                  "Bad query; sqlite error: ");
        }
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
        } else {
            throw IOError("RemoveRoutingTablesMigration::drop rt2 "
                                  "Run query; sqlite error: " + to_string(rc));
        }

        query = "DROP TABLE IF EXISTS rt3";
        rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);

        if (rc != SQLITE_OK) {
            throw IOError("RemoveRoutingTablesMigration::drop rt3 "
                                  "Bad query; sqlite error: ");
        }
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
        } else {
            throw IOError("RemoveRoutingTablesMigration::drop rt3 "
                                  "Run query; sqlite error: " + to_string(rc));
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);


    } catch(IOError& e){
        throw(e);
    }
}

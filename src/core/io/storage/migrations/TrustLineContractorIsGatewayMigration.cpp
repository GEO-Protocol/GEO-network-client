#include "TrustLineContractorIsGatewayMigration.h"

TrustLineContractorIsGatewayMigration::TrustLineContractorIsGatewayMigration(
    sqlite3 *dbConnection,
    Logger &logger):

    mDataBase(dbConnection),
    mLog(logger)
{}

void TrustLineContractorIsGatewayMigration::apply(IOTransaction::Shared ioTransaction)
{
    try {
        sqlite3_stmt *stmt;
        string query = "SELECT is_contractor_gateway FROM " + ioTransaction->trustLinesHandler()->tableName() + " LIMIT 1";
        auto rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            query = "alter table " + ioTransaction->trustLinesHandler()->tableName()
                    + " add column is_contractor_gateway INTEGER NOT NULL DEFAULT 0";
            rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
            if (rc != SQLITE_OK) {
                throw IOError("TrustLineContractorIsGatewayMigration::add new column is_contractor_gateway"
                                      "Bad query; sqlite error: ");
            }
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) {
            } else {
                throw IOError("TrustLineContractorIsGatewayMigration::add new column is_contractor_gateway: "
                                      "Run query; sqlite error: " + to_string(rc));
            }

            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
        }

    } catch(IOError& e){
        throw(e);
    }
}
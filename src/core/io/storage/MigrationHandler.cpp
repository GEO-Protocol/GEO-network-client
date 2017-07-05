#include "MigrationHandler.h"

MigrationHandler::MigrationHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        IOTransaction::Shared ioTransaction,
        Logger &logger):

        mDataBase(dbConnection),
        mTableName(tableName),
        mMigrationsSequence({}),
        mLog(logger)
{
    createTable();
    createMigrationsSequence();
}

const string MigrationHandler::logHeader() const
{
    stringstream s;
    s << "[MigrationHandler]";
    return s.str();
}

vector<MigrationUUID> MigrationHandler::allMigrationUUIDS() {
    vector<MigrationUUID> result;
    string query = "SELECT migration_uuid FROM "
                   + mTableName + ";";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("MigrationHandler::allMigrationUUIDS: "
                              "Bad query; sqlite error: " + rc);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        MigrationUUID stepMigrationUUID((uint8_t *)sqlite3_column_blob(stmt, 0));
        result.push_back(stepMigrationUUID);
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

int MigrationHandler::applyMigrations(
        IOTransaction::Shared ioTransaction)
{
    try {
        auto migrationsUUIDS = allMigrationUUIDS();
        for (auto const migrationUUID: mMigrationsSequence) {
            if (std::find(migrationsUUIDS.begin(), migrationsUUIDS.end(), migrationUUID) != migrationsUUIDS.end()) {
                info() << "Migration: " << migrationUUID << " already applied";
            } else {
                int stepStatus = applyMigration(migrationUUID, ioTransaction);
                if(stepStatus != 0){
                    return stepStatus;
                }
                info() << "Migration: " << migrationUUID << " successfully applied";
            }
        }

    } catch (const std::exception &e){
        mLog.logException("MigrationHandler", e);
        return -1;
    }
    return 0;
}

void MigrationHandler::createTable()
{
    sqlite3_stmt *stmt;
    // creating main table
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(migration_uuid BLOB NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("MigrationHandler::creating migration table: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("MigrationHandler::creating migration table: Run query; sqlite error: " + rc);
    }
    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_migration_uuid_idx on " + mTableName + "(migration_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("MigrationHandler::main creating index for migration_uuid: "
                              "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::main creating index for operation_uuid: "
                              "Run query; sqlite error: " + rc);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

LoggerStream MigrationHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream MigrationHandler::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream MigrationHandler::error() const
{
    return mLog.error(logHeader());
}

void MigrationHandler::createMigrationsSequence()
{
    // Amounts and Balances serializations
    mMigrationsSequence.push_back(MigrationUUID(string("0a889a5b-1a82-44c7-8b85-59db6f60a12d")));
}

int MigrationHandler::applyMigration(
    MigrationUUID migrationUUID,
    IOTransaction::Shared ioTransaction)
{
    if (migrationUUID.stringUUID() == string("0a889a5b-1a82-44c7-8b85-59db6f60a12d")){
        auto stepMigration = make_shared<AmountAndBalanceSerializationMigration>(
                mDataBase,
                mLog
        );
        int stepStatus = stepMigration->apply(ioTransaction);
        if (stepStatus != 0){
            ioTransaction->rollback();
            return stepStatus;
        }
        saveMigration(migrationUUID);
        return stepStatus;
    }
    error() << "Cannot recognize migrationUUID: " << migrationUUID << ".";
    return -1;
}

void MigrationHandler::saveMigration(MigrationUUID migrationUUID)
{
    string query = "INSERT INTO " + mTableName +
                        "(migration_uuid) VALUES (?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("MigrationHandler::insert : "
                              "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_bind_blob(stmt, 1, migrationUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("MigrationHandler::insert : "
                              "Bad binding of Contractor; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("MigrationHandler::insert : "
                              "Run query; sqlite error: " + rc);
    }
}

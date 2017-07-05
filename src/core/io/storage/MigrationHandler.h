#ifndef GEO_NETWORK_CLIENT_MIGRATIONHANDLER_H
#define GEO_NETWORK_CLIENT_MIGRATIONHANDLER_H

#include "../../common/NodeUUID.h"
#include "migrations/MigrationUUID.h"

#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"

#include "IOTransaction.h"
#include "migrations/migrationAmountAndBalanceSerialization.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>
#include <memory>
#include <algorithm>


class MigrationHandler {

public:
    MigrationHandler(
            sqlite3 *dbConnection,
            const string &tableName,
            IOTransaction::Shared ioTransaction,
            Logger &logger);

    int applyMigrations(IOTransaction::Shared ioTransaction);

protected:
    vector <MigrationUUID> allMigrationUUIDS();

    void createTable();

    void createMigrationsSequence();

    int applyMigration(
            MigrationUUID migrationUUID,
            IOTransaction::Shared ioTransaction);

    void saveMigration(MigrationUUID migrationUUID);

protected:
    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream error() const;

    const string logHeader() const;

    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
    vector<MigrationUUID> mMigrationsSequence;
};

#endif //GEO_NETWORK_CLIENT_MIGRATIONHANDLER_H

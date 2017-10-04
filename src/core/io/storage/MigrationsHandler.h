#ifndef GEO_NETWORK_CLIENT_MIGRATIONHANDLER_H
#define GEO_NETWORK_CLIENT_MIGRATIONHANDLER_H

#include "IOTransaction.h"
#include "migrations/AmountAndBalanceSerialization.h"
#include "migrations/SolomonHistoryMigration.h"
#include "migrations/SolomonHistoryMigrationTwo.h"
#include "migrations/UniqueIndexHistoryMigration.h"
#include "migrations/MaxDemianMigration.h"
#include "migrations/MaxDemianSecondMigration.h"
#include "migrations/MaxDemianThirdMigration.h"
#include "migrations/DeleteSerializedTransactionsMigration.h"
#include "migrations/PositiveSignMigration.h"
#include "migrations/CommandUUIDMigration.h"

#include "../../common/NodeUUID.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"
#include "../../logger/Logger.h"

#include <list>
#include <vector>
#include <memory>
#include <algorithm>


/**
 * @brief MigrationUUID class
 *
 * Used for distinguishing migrations between each other.
 * Separate class is used to prevent collisions with NodeUUIDs and TransactionsUUIDs
 * (all of them are derived from the boost::uuid)
 */
class MigrationUUID:

    // At this moment MigrationUUID uses the same logic as NodeUUID.
    public NodeUUID {

public:
    using NodeUUID::NodeUUID;
};


/**
 * @brief MigrationsHandler class
 *
 * Migrations handler collects migrations, that must be applied to the storage.
 * For the MVP release, SQLite plays the role if the internal storage,
 * but in the decentralised version - separate blockchain will be used.
 *
 * Migrations are used for sequential storage updates.
 */
class MigrationsHandler {
public:
    MigrationsHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        const NodeUUID &nodeUUID,
        TrustLineHandler *trustLineHandler,
        HistoryStorage *historyStorage,
        PaymentOperationStateHandler *paymentOperationStorage,
        TransactionsHandler *transactionHandler,
        Logger &logger);

    void applyMigrations();

protected:
    void enshureMigrationsTable();

    vector<MigrationUUID> allMigrationsUUIDS();

    void applyMigration(
        MigrationUUID migrationUUID,
        IOTransaction::Shared ioTransaction);

    void saveMigration(
        MigrationUUID migrationUUID);

protected:
    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream error() const;

    const string logHeader() const;

protected:
    Logger &mLog;
    sqlite3 *mDataBase;

    TrustLineHandler *mTrustLineHandler;
    HistoryStorage *mHistoryStorage;
    PaymentOperationStateHandler *mPaymentOperationStateHandler;
    TransactionsHandler *mTransactionHandler;

    // Name of the migrations table in the DB.
    // This table is used for storing information about already applied migrations.
    string mTableName;
    NodeUUID mNodeUUID;
};

#endif //GEO_NETWORK_CLIENT_MIGRATIONHANDLER_H

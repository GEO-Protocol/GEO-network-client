#include "MigrationsHandler.h"

MigrationsHandler::MigrationsHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    const NodeUUID &nodeUUID,
    TrustLineHandler *trustLineHandler,
    HistoryStorage *historyStorage,
    PaymentOperationStateHandler *paymentOperationStorage,
    TransactionsHandler *transactionHandler,
    BlackListHandler *blackListHandler,
    NodeFeaturesHandler *nodeFeaturesHandler,
    Logger &logger):

    mLog(logger),
    mDataBase(dbConnection),
    mTableName(tableName),
    mNodeUUID(nodeUUID),
    mTransactionHandler(transactionHandler),
    mTrustLineHandler(trustLineHandler),
    mPaymentOperationStateHandler(paymentOperationStorage),
    mBlackListHandler(blackListHandler),
    mHistoryStorage(historyStorage),
    mNodeFeaturesHandler(nodeFeaturesHandler)
{
    enshureMigrationsTable();
}

/**
 * Creates table for storing apllied migrations, and unique index for thier UUIDs.
 * In case if this table is already present - does nothing.
 *
 * @throws IOError in case of error.
 */
void MigrationsHandler::enshureMigrationsTable()
{
    sqlite3_stmt *stmt;

    /*
     * Main table creation.
     */
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName + " (migration_uuid BLOB NOT NULL);";

    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "MigrationsHandler::enshureMigrationsTable: "
            "Can't create migrations table: sqlite error code: " + to_string(rc) + ".");
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError(
            "MigrationsHandler::enshureMigrationsTable: "
            "Can't create migrations table: sqlite error code: " + to_string(rc) + ".");
    }

    /*
     * Creating unique index for preventing duplicating of migrations UUIDs.
     */
    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName + "_migration_uuid_index on " + mTableName + "(migration_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "MigrationsHandler::enshureMigrationsTable: "
            "Can't create index for migrations table: sqlite error code: " + to_string(rc) + ".");
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError(
            "MigrationsHandler::enshureMigrationsTable: "
            "Can't create index for migrations table: sqlite error code: " + to_string(rc) + ".");
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

/**
 * @returns vector with UUIDs of all applied migrations.
 * Returns empty vector in case if no migration was applied in the past.
 */
vector<MigrationUUID> MigrationsHandler::allMigrationsUUIDS() {
    vector<MigrationUUID> migrationsUUIDs;
    string query = "SELECT migration_uuid FROM " + mTableName + ";";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "MigrationsHandler::allMigrationUUIDS:"
            "Can't select applied migrations list.  "
            "sqlite error code: " + to_string(rc));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MigrationUUID migrationUUID(
            static_cast<const uint8_t *>(
                sqlite3_column_blob(stmt, 0)));

        migrationsUUIDs.push_back(migrationUUID);
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return migrationsUUIDs;
}

/**
 * Attempts to apply all migrations, that are listed in the method body.
 * The order, in which migrations would be applied, corresponds to the order, in which they are placed in the code.
 * In case, if some migration was already applied in the past - it would be skipped.
 *
 * @param ioTransaction - IO Transaction in scope of which the migrations must be applied.
 *
 * @throws RuntimeError in case of internal migration error.
 * The exception message would describe the error more detailed.
 */
void MigrationsHandler::applyMigrations()
{
    list<MigrationUUID> fullMigrationsUUIDsList = {
        MigrationUUID("0a889a5b-1a82-44c7-8b85-59db6f60a12d"),
        MigrationUUID("bc04656c-9dbb-4bd7-afd5-5603cf44b85e"),
        MigrationUUID("149daff7-ff15-49d6-a121-e2eb37a37ef7"),
        MigrationUUID("d65438b6-f5c3-473c-8018-7dbf874c5bc4"),
        MigrationUUID("4160e78e-f7bf-4499-a63d-18e312590ddf"),
        MigrationUUID("3a91cc61-fa61-4726-b0b8-bf692a94a0b2"),
        MigrationUUID("74b8aa70-1df2-49d3-9586-7eccccce5472")
        // ...q
        // the rest migrations must be placed here.
    };
    if (mNodeUUID == NodeUUID("2136a78d-3cb0-488d-b1a6-e039c12689d0")){
        fullMigrationsUUIDsList.push_back(MigrationUUID("c9ff4864-6626-11e7-861a-d397d1112608"));
        fullMigrationsUUIDsList.push_back(MigrationUUID("de88c613-c3c0-4cce-95f5-a90d9e1c6566"));
    }

    if (mNodeUUID == NodeUUID("9e2e8dff-a102-449a-92aa-f6be725be291")){
        fullMigrationsUUIDsList.push_back(MigrationUUID("727813ca-c9e0-44be-bd17-258831ad60f1"));
        fullMigrationsUUIDsList.push_back(MigrationUUID("27bb4b82-5b16-493d-af1d-621caee390c4"));
        fullMigrationsUUIDsList.push_back(MigrationUUID("fe9cb4c2-0dc2-40ae-9b4b-d50bc85343f4"));
    }

    try {
        auto migrationsUUIDS = allMigrationsUUIDS();

        for (auto const &kMigrationUUID : fullMigrationsUUIDsList) {
            if (std::find(migrationsUUIDS.begin(), migrationsUUIDS.end(), kMigrationUUID) != migrationsUUIDS.end()) {
                info() << "Migration " << kMigrationUUID << " is already applied. Skipped.";
                continue;
            }

            auto ioTransaction = make_shared<IOTransaction>(
                mDataBase,
                mTrustLineHandler,
                mHistoryStorage,
                mPaymentOperationStateHandler,
                mTransactionHandler,
                mBlackListHandler,
                mNodeFeaturesHandler,
                mLog);

            try {

                applyMigration(kMigrationUUID, ioTransaction);
                info() << "Migration " << kMigrationUUID << " successfully applied.";

            } catch (Exception &e) {

                error() << "Migration " << kMigrationUUID << " can't be applied. Details: " << e.what();
                ioTransaction->rollback();

                throw RuntimeError(e.what());
            }
        }

    } catch (const Exception &e) {
        // allNodesUUIDS() might throw IOError.
        mLog.logException("MigrationsHandler", e);
        throw RuntimeError(e.what());
    }
}

/**
 * Tries to apply migration with received UUID.
 *
 * @param migrationUUID - UUID of the migration that must be applied.
 * @param ioTransaction - IO transactions in scope of which the operation must be performed.
 *
 * @throws ValueError - in case if received UUID is not present in expected UUIDs list.
 * @throws RuntimeError - in case of migration error.
 */
void MigrationsHandler::applyMigration(
    MigrationUUID migrationUUID,
    IOTransaction::Shared ioTransaction)
{

    try {
        if (migrationUUID.stringUUID() == string("0a889a5b-1a82-44c7-8b85-59db6f60a12d")) {
            auto migration = make_shared<AmountAndBalanceSerializationMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("c9ff4864-6626-11e7-861a-d397d1112608")){
            auto migration = make_shared<SolomonHistoryMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("bc04656c-9dbb-4bd7-afd5-5603cf44b85e")){
            auto migration = make_shared<UniqueIndexHistoryMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("de88c613-c3c0-4cce-95f5-a90d9e1c6566")){
            auto migration = make_shared<SolomonHistoryMigrationTwo>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("727813ca-c9e0-44be-bd17-258831ad60f1")){
            auto migration = make_shared<MaxDemianMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("27bb4b82-5b16-493d-af1d-621caee390c4")){
            auto migration = make_shared<MaxDemianSecondMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("fe9cb4c2-0dc2-40ae-9b4b-d50bc85343f4")){
            auto migration = make_shared<MaxDemianThirdMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("d65438b6-f5c3-473c-8018-7dbf874c5bc4")){
            auto migration = make_shared<DeleteSerializedTransactionsMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("149daff7-ff15-49d6-a121-e2eb37a37ef7")){
            auto migration = make_shared<PositiveSignMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);
        } else if (migrationUUID.stringUUID() == string("4160e78e-f7bf-4499-a63d-18e312590ddf")) {
            auto migration = make_shared<CommandUUIDMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("3a91cc61-fa61-4726-b0b8-bf692a94a0b2")) {
            auto migration = make_shared<RemoveRoutingTablesMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

        } else if (migrationUUID.stringUUID() == string("74b8aa70-1df2-49d3-9586-7eccccce5472")){
            auto migration = make_shared<TrustLineContractorIsGatewayMigration>(
                mDataBase,
                mLog);

            migration->apply(ioTransaction);
            saveMigration(migrationUUID);

//            // ...
//            // Other migrations must be placed here
//            //
        } else {
            throw ValueError(
                "MigrationsHandler::applyMigration: "
                "can't recognize received migration UUID.");
        }
    } catch (const Exception &e) {
        // todo add e.what for info exception
        throw RuntimeError(e.what());
//        throw RuntimeError(
//            "MigrationsHandler::applyMigration: "
//            "Migration can't be applied. Details are: ");
    }
}

void MigrationsHandler::saveMigration(
    MigrationUUID migrationUUID)
{
    string query = "INSERT INTO " + mTableName +
                        "(migration_uuid) VALUES (?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("MigrationsHandler::insert : "
                              "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, migrationUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("MigrationsHandler::insert : "
                              "Bad binding of Contractor; sqlite error: "+ to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("MigrationsHandler::insert : "
                              "Run query; sqlite error: "+ to_string(rc));
    }
}

const string MigrationsHandler::logHeader() const
{
    stringstream s;
    s << "[MigrationsHandler]";
    return s.str();
}

LoggerStream MigrationsHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream MigrationsHandler::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream MigrationsHandler::error() const
{
    return mLog.warning(logHeader());
}



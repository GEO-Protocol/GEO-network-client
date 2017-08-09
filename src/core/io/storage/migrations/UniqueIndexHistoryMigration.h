#ifndef GEO_NETWORK_CLIENT_UNIQUEINDEXHISTORYMIGRATION_H
#define GEO_NETWORK_CLIENT_UNIQUEINDEXHISTORYMIGRATION_H

#include "AbstractMigration.h"

#include "../record/base/Record.h"
#include "../../../../libs/sqlite3/sqlite3.h"

#include <string>


class UniqueIndexHistoryMigration:
    public AbstractMigration {

public:
    UniqueIndexHistoryMigration(
        sqlite3 *dbConnection,
        Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);

protected:
    void migrateHistory(
        IOTransaction::Shared ioTransaction);

    void migrateHistoryAdditional(
        IOTransaction::Shared ioTransaction);

protected:
    sqlite3 *mDataBase;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_UNIQUEINDEXHISTORYMIGRATION_H

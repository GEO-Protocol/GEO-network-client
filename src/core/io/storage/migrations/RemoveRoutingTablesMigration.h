#ifndef GEO_NETWORK_CLIENT_REMOVEROUTINGTABLESMIGRATION_H
#define GEO_NETWORK_CLIENT_REMOVEROUTINGTABLESMIGRATION_H

#include "AbstractMigration.h"

class RemoveRoutingTablesMigration : public AbstractMigration {

public:
    RemoveRoutingTablesMigration(
        sqlite3 *dbConnection,
        Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);

protected:
    sqlite3 *mDataBase;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_REMOVEROUTINGTABLESMIGRATION_H

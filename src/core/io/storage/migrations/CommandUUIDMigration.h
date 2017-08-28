#ifndef GEO_NETWORK_CLIENT_COMMANDUUIDMIGRATION_H
#define GEO_NETWORK_CLIENT_COMMANDUUIDMIGRATION_H


#include "AbstractMigration.h"

#include "../record/base/Record.h"
#include "../../../../libs/sqlite3/sqlite3.h"

#include <string>


class CommandUUIDMigration:
        public AbstractMigration {

public:
    CommandUUIDMigration(
            sqlite3 *dbConnection,
            Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);

protected:
    sqlite3 *mDataBase;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_COMMANDUUIDMIGRATION_H

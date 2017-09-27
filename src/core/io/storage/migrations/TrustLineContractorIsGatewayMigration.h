#ifndef GEO_NETWORK_CLIENT_TRUSTLINECONTRACTORISGATEWAYMIGRATION_H
#define GEO_NETWORK_CLIENT_TRUSTLINECONTRACTORISGATEWAYMIGRATION_H

#include "AbstractMigration.h"

#include "../../../../libs/sqlite3/sqlite3.h"

class TrustLineContractorIsGatewayMigration : public AbstractMigration {

public:
    TrustLineContractorIsGatewayMigration(
        sqlite3 *dbConnection,
        Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);

protected:
    sqlite3 *mDataBase;
    Logger &mLog;

};


#endif //GEO_NETWORK_CLIENT_TRUSTLINECONTRACTORISGATEWAYMIGRATION_H

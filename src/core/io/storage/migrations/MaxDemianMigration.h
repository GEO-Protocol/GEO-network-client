#ifndef GEO_NETWORK_CLIENT_MAXDEMIANMIGRATION_H
#define GEO_NETWORK_CLIENT_MAXDEMIANMIGRATION_H

#include "AbstractMigration.h"
#include "../../../transactions/transactions/base/TransactionUUID.h"


class MaxDemianMigration:
    public AbstractMigration {

public:
    MaxDemianMigration(
        sqlite3 *dbConnection,
        Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);

protected:
    TrustLine::Shared getOutwornTrustLine(
        IOTransaction::Shared ioTransaction,
        NodeUUID &nodeUUID);

protected:
    sqlite3 *mDataBase;
    Logger &mLog;
    // Amount on which was rollbacked operation
    TrustLineAmount mOperationAmount;
    // Neighbor with which was reservation and rollBack
    NodeUUID mMigrateNodeUUID;
    // CoordinatorUUID;
    NodeUUID mNeighborUUIDDebtor;
    // Neighbor UUID
    NodeUUID mNeighborUUIDCreditor;
};


#endif //GEO_NETWORK_CLIENT_MAXDEMIANMIGRATION_H

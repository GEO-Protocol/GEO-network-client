#ifndef GEO_NETWORK_CLIENT_SOLOMONHISTORYMIGRATION_H
#define GEO_NETWORK_CLIENT_SOLOMONHISTORYMIGRATION_H

#include "AbstractMigration.h"
#include "../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"



class SolomonHistoryMigration:
    public AbstractMigration {

public:
    SolomonHistoryMigration(
        sqlite3 *dbConnection,
        Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);


protected:
    void applyTrustLineMigration(IOTransaction::Shared ioTransaction);
    void applyTrustLineHistoryMigration(IOTransaction::Shared ioTransaction);

public:
    TrustLine::Shared getOutwornTrustLine(
        IOTransaction::Shared ioTransaction);

protected:
    sqlite3 *mDataBase;
    Logger &mLog;
    // Amount on which was rollbacked operation
    TrustLineBalance mOperationAmount;
    // Operation UUID that was rollbaked
    TransactionUUID mOperationUUID;
    // Operation UUID that was commited before rollbacked operation
    TransactionUUID mPreviousOperationUUID;
    // Neighbor with which was reservation and rollBack
    NodeUUID mMigrateNodeUUID;
    // CoordinatorUUID;
    NodeUUID mCoordinatorUUID;
    // Neighbor UUID
    NodeUUID mNeighborUUID;

};


#endif //GEO_NETWORK_CLIENT_SOLOMONHISTORYMIGRATION_H
#ifndef GEO_NETWORK_CLIENT_SOLOMONHISTORYMIGRATIONTWO_H
#define GEO_NETWORK_CLIENT_SOLOMONHISTORYMIGRATIONTWO_H

#include "AbstractMigration.h"
#include "../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"



class SolomonHistoryMigrationTwo:
    public AbstractMigration {

public:
    SolomonHistoryMigrationTwo(
        sqlite3 *dbConnection,
        Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);

protected:
    void applyTrustLineMigration(IOTransaction::Shared ioTransaction);
    void applyTrustLineHistoryMigration(IOTransaction::Shared ioTransaction);

public:
    TrustLine::Shared getOutwornTrustLine(
        IOTransaction::Shared ioTransaction);

    PaymentRecord::Shared getPreviousPaymentRecord(
        IOTransaction::Shared ioTransaction);

    PaymentRecord::Shared deserializePaymentRecord(
        sqlite3_stmt *stmt);

    vector<PaymentRecord::Shared> getPaymentRecordsForUpdate(
        IOTransaction::Shared ioTransaction,
        int64_t since_what_timestamp);

    void updatePaymentsRecords(
        IOTransaction::Shared ioTransaction,
        vector<PaymentRecord::Shared> paymentsRecordsToUpdate);

    pair<BytesShared, size_t> serializedPaymentRecordBody(
        PaymentRecord::Shared paymentRecord);

protected:
    sqlite3 *mDataBase;
    Logger &mLog;
    // Amount on which was rollbacked operation
    TrustLineAmount mOperationAmount;
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
    GEOEpochTimestamp mOperationTimeStampOnCoordinatorNode;

};

#endif //GEO_NETWORK_CLIENT_SOLOMONHISTORYMIGRATIONTWO_H

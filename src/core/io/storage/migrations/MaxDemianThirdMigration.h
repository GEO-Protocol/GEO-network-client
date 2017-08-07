#ifndef GEO_NETWORK_CLIENT_MAXDEMIANTHIRDMIGRATION_H
#define GEO_NETWORK_CLIENT_MAXDEMIANTHIRDMIGRATION_H

#include "AbstractMigration.h"
#include "../../../transactions/transactions/base/TransactionUUID.h"


class MaxDemianThirdMigration:
    public AbstractMigration {

public:
    MaxDemianThirdMigration(
        sqlite3 *dbConnection,
        Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);

protected:
    void applyTrustLineMigration(IOTransaction::Shared ioTransaction);
    void applyTrustLineHistoryMigration(IOTransaction::Shared ioTransaction);

protected:
    TrustLine::Shared getOutwornTrustLine(
        IOTransaction::Shared ioTransaction,
        NodeUUID &nodeUUID);

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

    // Nodes which wit was rollbacked operation
    NodeUUID mNeighborUUID;

    GEOEpochTimestamp mOperationTimeStampOnCoordinatorNode;
};


#endif //GEO_NETWORK_CLIENT_MAXDEMIANTHIRDMIGRATION_H

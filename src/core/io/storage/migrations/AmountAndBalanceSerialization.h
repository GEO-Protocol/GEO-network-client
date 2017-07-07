#ifndef GEO_NETWORK_CLIENT_MIGRATIONAMOUNTANDBALANCESERIALIZATION_H
#define GEO_NETWORK_CLIENT_MIGRATIONAMOUNTANDBALANCESERIALIZATION_H

#include "AbstractMigration.h"

#include "../record/base/Record.h"
#include "../../../../libs/sqlite3/sqlite3.h"

#include <string>


class AmountAndBalanceSerializationMigration:
    public AbstractMigration {

public:
    AmountAndBalanceSerializationMigration(
        sqlite3 *dbConnection,
        Logger &logger);



    void apply(IOTransaction::Shared ioTransaction);

protected:
    void migrateTrustLines(
            IOTransaction::Shared ioTransaction);

    void migrateTrustLinesHistory(
            IOTransaction::Shared ioTransaction);

    void migratePaymentsHistory(
            IOTransaction::Shared ioTransaction);

    vector<PaymentRecord::Shared> allMainPaymentRecords(
            IOTransaction::Shared ioTransaction);

    vector<PaymentRecord::Shared> allAdditionalPaymentRecords(
            IOTransaction::Shared ioTransaction);

    vector<TrustLineRecord::Shared> allTrustLinesRecords(
            IOTransaction::Shared ioTransaction);

    PaymentRecord::Shared deserializePaymentRecordOld(
            sqlite3_stmt *stmt);

    PaymentRecord::Shared deserializePaymentAdditionalRecordOld(
            sqlite3_stmt *stmt);

    TrustLineRecord::Shared deserializeTrustLineRecordOld(
            sqlite3_stmt *stmt);

    void updateMainPaymentRecord(
            PaymentRecord::Shared paymentRecord,
            IOTransaction::Shared ioTransaction);

    void updateAdditionalPaymentRecord(
            PaymentRecord::Shared paymentRecord,
            IOTransaction::Shared ioTransaction);

    void updateTrustLineRecord(
            TrustLineRecord::Shared trustLineRecord,
            IOTransaction::Shared ioTransaction);

protected:
    pair<BytesShared, size_t> serializedMainPaymentRecordBody(
            PaymentRecord::Shared paymentRecord);

    pair<BytesShared, size_t> serializedAdditionalPaymentRecordBody(
            PaymentRecord::Shared paymentRecord);

    pair<BytesShared, size_t> serializedTrustLineRecordBody(
            TrustLineRecord::Shared trustLineRecord);

    TrustLineBalance bytesToTrustLineBalanceOld(const vector<byte> balanceBytes);

    TrustLineAmount bytesToTrustLineAmountOld(const vector<byte> &amountBytes);

protected:
    sqlite3 *mDataBase = nullptr;
    Logger &mLog;
};

#endif //GEO_NETWORK_CLIENT_MIGRATIONAMOUNTANDBALANCESERIALIZATION_H

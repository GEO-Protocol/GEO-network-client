#ifndef GEO_NETWORK_CLIENT_POSITIVESIGNMIGRATION_H
#define GEO_NETWORK_CLIENT_POSITIVESIGNMIGRATION_H

#include "AbstractMigration.h"

#include "../record/base/Record.h"
#include "../../../../libs/sqlite3/sqlite3.h"

#include <string>


class PositiveSignMigration:
    public AbstractMigration {

public:
    PositiveSignMigration(
        sqlite3 *dbConnection,
        Logger &logger);

    void apply(IOTransaction::Shared ioTransaction);

protected:
    void migrateTrustLines(
        IOTransaction::Shared ioTransaction);

    void migratePaymentsHistory(
        IOTransaction::Shared ioTransaction);

    vector<PaymentRecord::Shared> allMainPaymentRecords(
        IOTransaction::Shared ioTransaction);

    PaymentRecord::Shared deserializePaymentRecordOld(
        sqlite3_stmt *stmt);

    void updateMainPaymentRecord(
        PaymentRecord::Shared paymentRecord,
        IOTransaction::Shared ioTransaction);

protected:
    pair<BytesShared, size_t> serializedMainPaymentRecordBody(
        PaymentRecord::Shared paymentRecord);

    TrustLineBalance bytesToTrustLineBalanceOld(const vector<byte> balanceBytes);

protected:
    sqlite3 *mDataBase;
    Logger &mLog;
};

#endif //GEO_NETWORK_CLIENT_POSITIVESIGNMIGRATION_H

#ifndef GEO_NETWORK_CLIENT_DELETESERIALIZEDTRANSACTIONSMIGRATION_H
#define GEO_NETWORK_CLIENT_DELETESERIALIZEDTRANSACTIONSMIGRATION_H

#include "AbstractMigration.h"

class DeleteSerializedTransactionsMigration:
    public AbstractMigration {

public:
    DeleteSerializedTransactionsMigration(
        sqlite3 *dbConnection,
    Logger &logger);

public:
    void apply(IOTransaction::Shared ioTransaction);


protected:
    sqlite3 *mDataBase;
    Logger &mLog;

    TransactionUUID mTransactionUUID;
};

#endif //GEO_NETWORK_CLIENT_DELETESERIALIZEDTRANSACTIONSMIGRATION_H

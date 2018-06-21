#ifndef GEO_NETWORK_CLIENT_PAYMENTKEYSHANDLER_H
#define GEO_NETWORK_CLIENT_PAYMENTKEYSHANDLER_H

#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../crypto/lamportkeys.h"

#include "../../../libs/sqlite3/sqlite3.h"

using namespace crypto::lamport;

class PaymentKeysHandler {

public:
    PaymentKeysHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveOwnKey(
        const TransactionUUID &transactionUUID,
        const NodeUUID &ownNodeUUID,
        const PublicKey::Shared publicKey,
        const PrivateKey *privateKey);

    PrivateKey* getOwnPrivateKey(
        const TransactionUUID &transactionUUID);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_PAYMENTKEYSHANDLER_H

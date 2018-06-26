#ifndef GEO_NETWORK_CLIENT_CONTRACTORKEYSHANDLER_H
#define GEO_NETWORK_CLIENT_CONTRACTORKEYSHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../crypto/lamportkeys.h"

#include "../../../libs/sqlite3/sqlite3.h"

using namespace crypto::lamport;

class ContractorKeysHandler {

public:
    ContractorKeysHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveKey(
        const TrustLineID trustLineID,
        const PublicKey::Shared publicKey,
        const KeyNumber number);

    PublicKey::Shared keyByNumber(
        const TrustLineID trustLineID,
        const KeyNumber number);

    const KeyHash::Shared keyHashByNumber(
        const TrustLineID trustLineID,
        const KeyNumber number);

    KeysCount availableKeysCnt(
        const TrustLineID trustLineID);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTORKEYSHANDLER_H

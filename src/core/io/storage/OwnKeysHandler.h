#ifndef GEO_NETWORK_CLIENT_OWNKEYSHANDLER_H
#define GEO_NETWORK_CLIENT_OWNKEYSHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../crypto/CryptoKey.h"

#include "../../../libs/sqlite3/sqlite3.h"
#include <vector>

class OwnKeysHandler {

public:
    OwnKeysHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveKey(
        const TrustLineID trustLineID,
        const CryptoKey &publicKey,
        const CryptoKey &privateKey,
        uint32_t number);

    pair<uint32_t, CryptoKey> nextAvailableKey(
        const TrustLineID trustLineID);

    void invalidKey(
        const TrustLineID trustLineID,
        uint32_t number);

    vector<pair<uint32_t, CryptoKey>> allAvailablePublicKeys(
        const TrustLineID trustLineID);

    uint32_t availableKeysCnt(
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


#endif //GEO_NETWORK_CLIENT_OWNKEYSHANDLER_H

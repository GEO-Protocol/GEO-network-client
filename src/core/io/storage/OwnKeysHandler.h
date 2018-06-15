#ifndef GEO_NETWORK_CLIENT_OWNKEYSHANDLER_H
#define GEO_NETWORK_CLIENT_OWNKEYSHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../crypto/lamportkeys.h"

#include "../../../libs/sqlite3/sqlite3.h"

using namespace crypto::lamport;

class OwnKeysHandler {

public:
    OwnKeysHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveKey(
        const TrustLineID trustLineID,
        const PublicKey::Shared publicKey,
        const PrivateKey *privateKey,
        const KeyNumber number);

    pair<PrivateKey*, KeyNumber> nextAvailableKey(
        const TrustLineID trustLineID);

    void invalidKey(
        const TrustLineID trustLineID,
        const KeyNumber number);

    const PublicKey::Shared getPublicKey(
        const TrustLineID trustLineID,
        const KeyNumber keyNumber);

    const uint32_t getPublicKeyHash(
        const TrustLineID trustLineID,
        const KeyNumber keyNumber);

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


#endif //GEO_NETWORK_CLIENT_OWNKEYSHANDLER_H

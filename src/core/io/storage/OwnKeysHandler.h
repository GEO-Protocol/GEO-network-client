#ifndef GEO_NETWORK_CLIENT_OWNKEYSHANDLER_H
#define GEO_NETWORK_CLIENT_OWNKEYSHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../crypto/lamportkeys.h"
#include "../../crypto/lamportscheme.h"
#include "../../common/memory/MemoryUtils.h"

#include "../../../libs/sqlite3/sqlite3.h"

using namespace crypto;

class OwnKeysHandler {

public:
    OwnKeysHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveKey(
        const TrustLineID trustLineID,
        const lamport::PublicKey::Shared publicKey,
        const lamport::PrivateKey *privateKey,
        const KeyNumber number);

    pair<lamport::PrivateKey*, KeyNumber> nextAvailableKey(
        const TrustLineID trustLineID);

    void invalidKey(
        const TrustLineID trustLineID,
        const KeyNumber number,
        const lamport::Signature::Shared signature);

    void invalidKeyByHash(
        const TrustLineID trustLineID,
        const lamport::KeyHash::Shared keyHash,
        const lamport::Signature::Shared signature);

    const lamport::PublicKey::Shared getPublicKey(
        const TrustLineID trustLineID,
        const KeyNumber keyNumber);

    const lamport::PublicKey::Shared getPublicKeyByHash(
        const TrustLineID trustLineID,
        const lamport::KeyHash::Shared keyHash);

    const lamport::KeyHash::Shared getPublicKeyHash(
        const TrustLineID trustLineID,
        const KeyNumber keyNumber);

    KeysCount availableKeysCnt(
        const TrustLineID trustLineID);

    void removeUnusedKeys(
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

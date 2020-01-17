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

using namespace crypto::lamport;

class OwnKeysHandler {

public:
    OwnKeysHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveKey(
        const TrustLineID trustLineID,
        const KeyNumber keysSetSequenceNumber,
        const PublicKey::Shared publicKey,
        const PrivateKey *privateKey,
        const KeyNumber number);

    const KeyNumber maxKeySetSequenceNumber(
        const TrustLineID trustLineID);

    pair<PrivateKey*, KeyNumber> nextAvailableKey(
        const TrustLineID trustLineID);

    void invalidKey(
        const TrustLineID trustLineID,
        const KeyNumber number,
        const Signature::Shared signature);

    void invalidKeyByHash(
        const TrustLineID trustLineID,
        const KeyHash::Shared keyHash,
        const Signature::Shared signature);

    const PublicKey::Shared getPublicKey(
        const TrustLineID trustLineID,
        const KeyNumber keyNumber);

    const PublicKey::Shared getPublicKeyByHash(
        const TrustLineID trustLineID,
        const KeyHash::Shared keyHash);

    const KeyHash::Shared getPublicKeyHash(
        const TrustLineID trustLineID,
        const KeyNumber keyNumber);

    const KeyNumber getKeyNumberByHash(
        const KeyHash::Shared keyHash);

    KeysCount availableKeysCnt(
        const TrustLineID trustLineID);

    void removeUnusedKeys(
        const TrustLineID trustLineID);

    vector<PublicKey::Shared> publicKeysBySetNumber(
        const TrustLineID trustLineID,
        const KeyNumber keysSetSequenceNumber) const;

    void deleteKeysByTrustLineID(
        const TrustLineID trustLineID);

    void deleteKeyByHashExceptSequenceNumber(
            KeyHash::Shared keyHash,
            const KeyNumber keysSetSequenceNumber);

    vector<KeyHash::Shared> publicKeyHashesLessThanSetNumber(
        const TrustLineID trustLineID,
        const KeyNumber keysSetSequenceNumber) const;

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

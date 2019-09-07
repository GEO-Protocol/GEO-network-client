#include "OwnKeysHandler.h"

OwnKeysHandler::OwnKeysHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (hash BLOB PRIMARY KEY, "
                   "trust_line_id INTEGER NOT NULL, "
                   "keys_set_sequence_number INTEGER NOT NULL, "
                   "public_key BLOB NOT NULL, "
                   "private_key BLOB NOT NULL, "
                   "number INTEGER NOT NULL, "
                   "is_valid INTEGER NOT NULL DEFAULT 1, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("OwnKeysHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_hash_idx on " + mTableName + "(hash);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::creating  index for Hash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("OwnKeysHandler::creating index for Hash: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_trust_line_id_idx on " + mTableName + "(trust_line_id);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::creating  index for Trust Line ID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("OwnKeysHandler::creating index for Trust Line ID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void OwnKeysHandler::saveKey(
    const TrustLineID trustLineID,
    const KeyNumber keysSetSequenceNumber,
    const PublicKey::Shared publicKey,
    const PrivateKey *privateKey,
    const KeyNumber number) {
    string query = "INSERT INTO " + mTableName +
                   "(hash, trust_line_id, keys_set_sequence_number, public_key, private_key, number) "
                           "VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, publicKey->hash()->data(),
                           (int) KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Hash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, keysSetSequenceNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Key Set Sequence Number; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 4, publicKey->data(),
                           (int) publicKey->keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Public Key; sqlite error: " + to_string(rc));
    }

    // todo encrypt private key data
    BytesShared buffer = tryMalloc(
        PrivateKey::keySize());
    {
        auto g = privateKey->data()->unlockAndInitGuard();
        memcpy(
            buffer.get(),
            g.address(),
            PrivateKey::keySize());
    }
    rc = sqlite3_bind_blob(stmt, 5, buffer.get(),
                           (int)PrivateKey::keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Private Key; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 6, number);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }
    // todo : add saving of is_valid

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting or replacing is completed successfully";
#endif
    } else {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

const KeyNumber OwnKeysHandler::maxKeySetSequenceNumber(
    const TrustLineID trustLineID)
{
    string query = "SELECT MAX(keys_set_sequence_number) FROM " + mTableName
                   + " WHERE trust_line_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::maxKeySetSequenceNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::maxKeySetSequenceNumber: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // todo : check if not NULL
        auto result = (KeyNumber)sqlite3_column_int(stmt, 0);
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("OwnKeysHandler::maxKeySetSequenceNumber: "
                                "There are now records with requested TrustLineID");
    }
}

pair<PrivateKey*, KeyNumber> OwnKeysHandler::nextAvailableKey(
    const TrustLineID trustLineID)
{
    string query = "SELECT private_key, number FROM " + mTableName
                   + " WHERE trust_line_id = ? AND is_valid = 1 ORDER BY number ASC LIMIT 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::nextAvailableKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::nextAvailableKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto privateKey = new PrivateKey((byte*)sqlite3_column_blob(stmt, 0));
        auto number = (KeyNumber)sqlite3_column_int(stmt, 1);
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return make_pair(
            privateKey,
            number);
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("OwnKeysHandler::nextAvailableKey: "
                                "There are now records with requested trust line id");
    }
}

void OwnKeysHandler::invalidKey(
    const TrustLineID trustLineID,
    const KeyNumber number,
    const Signature::Shared signature)
{
    string query = "UPDATE " + mTableName + " SET is_valid = 0, private_key = ? WHERE trust_line_id = ? AND number = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, signature->data(), (int)signature->signatureSize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKey: "
                          "Bad binding of Private key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, number);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKey: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("OwnKeysHandler::invalidKey: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were changed");
    }
}

void OwnKeysHandler::invalidKeyByHash(
    const TrustLineID trustLineID,
    const KeyHash::Shared keyHash,
    const Signature::Shared signature)
{
    string query = "UPDATE " + mTableName + " SET is_valid = 0, private_key = ? WHERE trust_line_id = ? AND hash = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKeyByHash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, signature->data(), (int)signature->signatureSize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKeyByHash: "
                          "Bad binding of Private key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKeyByHash: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, keyHash->data(), (int)KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKeyByHash: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("OwnKeysHandler::invalidKeyByHash: "
                              "Run query; sqlite error: " + to_string(rc));
    }

    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were changed");
    }
}

const PublicKey::Shared OwnKeysHandler::getPublicKey(
    const TrustLineID trustLineID,
    const KeyNumber keyNumber)
{
    string query = "SELECT public_key FROM  " + mTableName
                   + " WHERE trust_line_id = ? AND number = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, keyNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKey: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto result = make_shared<PublicKey>(
            (byte*)sqlite3_column_blob(stmt, 0));
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("OwnKeysHandler::getPublicKey: "
                                "There are now records with requested trust line id");
    }
}

const PublicKey::Shared OwnKeysHandler::getPublicKeyByHash(
    const TrustLineID trustLineID,
    const KeyHash::Shared keyHash)
{
    string query = "SELECT public_key FROM  " + mTableName
                   + " WHERE trust_line_id = ? AND hash = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyByHash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyByHash: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, keyHash->data(), (int) KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyByHash: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto result = make_shared<PublicKey>(
                (byte*)sqlite3_column_blob(stmt, 0));
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("OwnKeysHandler::getPublicKeyByHash: "
                                "There are now records with requested trust line id");
    }
}

const KeyHash::Shared OwnKeysHandler::getPublicKeyHash(
    const TrustLineID trustLineID,
    const KeyNumber keyNumber)
{
    string query = "SELECT hash FROM  " + mTableName
                   + " WHERE trust_line_id = ? AND number = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyHash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyHash: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, keyNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyHash: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto result = make_shared<KeyHash>(
            (byte*)sqlite3_column_blob(stmt, 0));
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("OwnKeysHandler::getPublicKeyHash: "
                                "There are now records with requested trust line id");
    }
}

const KeyNumber OwnKeysHandler::getKeyNumberByHash(
    const KeyHash::Shared keyHash)
{
    string query = "SELECT number FROM  " + mTableName + " WHERE hash = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getKeyNumberByHash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, keyHash->data(), (int) KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getKeyNumberByHash: "
                          "Bad binding of Hash; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto result = (KeyNumber)sqlite3_column_int(stmt, 0);
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("OwnKeysHandler::getKeyNumberByHash: "
                                "There are now records with requested hash");
    }
}

KeysCount OwnKeysHandler::availableKeysCnt(
    const TrustLineID trustLineID)
{
    string queryCount = "SELECT count(*) FROM " + mTableName + " WHERE trust_line_id = ? AND is_valid = 1";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::availableKeysCnt: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::availableKeysCnt: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (KeysCount)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return rowCount;
}

void OwnKeysHandler::removeUnusedKeys(
    const TrustLineID trustLineID)
{
    string queryCount = "DELETE FROM " + mTableName + " WHERE trust_line_id = ? AND is_valid = 1";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::removeUnusedKeys: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::removeUnusedKeys: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("OwnKeysHandler::removeUnusedKeys: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

vector<PublicKey::Shared> OwnKeysHandler::publicKeysBySetNumber(
    const TrustLineID trustLineID,
    const KeyNumber keysSetSequenceNumber) const
{
    vector<PublicKey::Shared> result;
    string queryCount = "SELECT count(*) FROM " + mTableName
            + " WHERE trust_line_id = ? AND keys_set_sequence_number = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::publicKeysBySetNumber: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::publicKeysBySetNumber: "
                          "Bad binding of Trust Line ID in count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, keysSetSequenceNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::publicKeysBySetNumber: "
                          "Bad binding of Keys Set Sequence Number in count query; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    result.reserve(rowCount);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    string query = "SELECT public_key FROM "
                   + mTableName + " WHERE trust_line_id = ? AND keys_set_sequence_number = ?";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::publicKeysBySetNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::publicKeysBySetNumber: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, keysSetSequenceNumber);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::publicKeysBySetNumber: "
                          "Bad binding of Keys Set Sequence Number; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        auto publicKey = make_shared<PublicKey>(
            (byte*)sqlite3_column_blob(stmt, 0));
        result.push_back(publicKey);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

void OwnKeysHandler::deleteKeysByTrustLineID(
    const TrustLineID trustLineID)
{
    string query = "DELETE FROM " + mTableName + " WHERE trust_line_id = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::deleteKeysByTrustLineID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::deleteKeysByTrustLineID: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("OwnKeysHandler::deleteKeysByTrustLineID: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}


LoggerStream OwnKeysHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream OwnKeysHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string OwnKeysHandler::logHeader() const
{
    stringstream s;
    s << "[OwnKeysHandler]";
    return s.str();
}
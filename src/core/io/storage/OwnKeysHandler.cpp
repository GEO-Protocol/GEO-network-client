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
                   "public_key BLOB NOT NULL, "
                   "private_key BLOB NOT NULL, "
                   "number INTEGER NOT NULL, "
                   "is_valid INTEGER NOT NULL DEFAULT 1, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
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
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
    const lamport::PublicKey::Shared publicKey,
    const lamport::PrivateKey *privateKey,
    const KeyNumber number) {
    string query = "INSERT INTO " + mTableName +
                   "(hash, trust_line_id, public_key, private_key, number) "
                           "VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                              "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, publicKey->hash()->data(),
                           (int) lamport::KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                              "Bad binding of Hash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                              "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, publicKey->data(),
                           (int) publicKey->keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                              "Bad binding of Public Key; sqlite error: " + to_string(rc));
    }

    // todo encrypt private key data
    BytesShared buffer = tryMalloc(privateKey->keySize());
    {
        auto g = privateKey->data()->unlockAndInitGuard();
        memcpy(
            buffer.get(),
            g.address(),
            privateKey->keySize());
    }
    rc = sqlite3_bind_blob(stmt, 4, buffer.get(),
                           (int)privateKey->keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                              "Bad binding of Private Key; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 5, number);
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

pair<lamport::PrivateKey*, KeyNumber> OwnKeysHandler::nextAvailableKey(
    const TrustLineID trustLineID)
{
    string query = "SELECT private_key, number FROM " + mTableName
                   + " WHERE trust_line_id = ? AND is_valid = 1 ORDER BY number ASC LIMIT 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
        auto privateKey = new lamport::PrivateKey((byte*)sqlite3_column_blob(stmt, 0));
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
    const lamport::Signature::Shared signature)
{
    string query = "UPDATE " + mTableName + " SET is_valid = 0, private_key = ? WHERE trust_line_id = ? AND number = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
    const lamport::KeyHash::Shared keyHash,
    const lamport::Signature::Shared signature)
{
    string query = "UPDATE " + mTableName + " SET is_valid = 0, private_key = ? WHERE trust_line_id = ? AND hash = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
    rc = sqlite3_bind_blob(stmt, 3, keyHash->data(), (int)lamport::KeyHash::kBytesSize, SQLITE_STATIC);
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

const lamport::PublicKey::Shared OwnKeysHandler::getPublicKey(
    const TrustLineID trustLineID,
    const KeyNumber keyNumber)
{
    string query = "SELECT public_key FROM  " + mTableName
                   + " WHERE trust_line_id = ? AND number = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
        auto result = make_shared<lamport::PublicKey>(
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

const lamport::PublicKey::Shared OwnKeysHandler::getPublicKeyByHash(
    const TrustLineID trustLineID,
    const lamport::KeyHash::Shared keyHash)
{
    string query = "SELECT public_key FROM  " + mTableName
                   + " WHERE trust_line_id = ? AND hash = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyByHash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyByHash: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, keyHash->data(), (int) lamport::KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::getPublicKeyByHash: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto result = make_shared<lamport::PublicKey>(
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

const lamport::KeyHash::Shared OwnKeysHandler::getPublicKeyHash(
    const TrustLineID trustLineID,
    const KeyNumber keyNumber)
{
    string query = "SELECT hash FROM  " + mTableName
                   + " WHERE trust_line_id = ? AND number = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
        auto result = make_shared<lamport::KeyHash>(
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

KeysCount OwnKeysHandler::availableKeysCnt(
    const TrustLineID trustLineID)
{
    string queryCount = "SELECT count(*) FROM " + mTableName + " WHERE trust_line_id = ? AND is_valid = 1";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, 0);
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
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, 0);
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
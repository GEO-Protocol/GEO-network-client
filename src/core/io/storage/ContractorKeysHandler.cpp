#include "ContractorKeysHandler.h"

ContractorKeysHandler::ContractorKeysHandler(
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
                   "number INTEGER NOT NULL, "
                   "is_valid INTEGER NOT NULL DEFAULT 1, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("ContractorKeysHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_hash_idx on " + mTableName + "(hash);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::creating  index for Hash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("ContractorKeysHandler::creating index for Hash: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_trust_line_id_idx on " + mTableName + "(trust_line_id);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::creating  index for Trust Line ID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("ContractorKeysHandler::creating index for Trust Line ID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void ContractorKeysHandler::saveKey(
    const TrustLineID trustLineID,
    const KeyNumber keysSetSequenceNumber,
    const PublicKey::Shared publicKey,
    const KeyNumber number)
{
    string query = "INSERT INTO " + mTableName +
                   "(hash, trust_line_id, keys_set_sequence_number, public_key, "
                   "number) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, publicKey->hash()->data(), (int)KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of Hash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, keysSetSequenceNumber);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of Keys Set Sequence Number; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 4, publicKey->data(),
                       (int)publicKey->keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of Public Key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 5, number);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }
    // todo : add saving of is_valid

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

const KeyNumber ContractorKeysHandler::maxKeySetSequenceNumber(
    const TrustLineID trustLineID)
{
    string query = "SELECT MAX(keys_set_sequence_number) FROM " + mTableName
                   + " WHERE trust_line_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::maxKeySetSequenceNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::maxKeySetSequenceNumber: "
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
        throw NotFoundError("ContractorKeysHandler::maxKeySetSequenceNumber: "
                                "There are now records with requested TrustLineID");
    }
}

void ContractorKeysHandler::invalidKey(
    const TrustLineID trustLineID,
    const KeyNumber number)
{
    string query = "UPDATE " + mTableName + " SET is_valid = 0 WHERE trust_line_id = ? AND number = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::invalidKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::invalidKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, number);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::invalidKey: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("ContractorKeysHandler::invalidKey: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were changed");
    }
}

void ContractorKeysHandler::invalidKeyByHash(
    const TrustLineID trustLineID,
    const KeyHash::Shared keyHash)
{
    string query = "UPDATE " + mTableName + " SET is_valid = 0 WHERE trust_line_id = ? AND number = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::invalidKeyByHash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::invalidKeyByHash: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, keyHash->data(), (int)KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::invalidKeyByHash: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("ContractorKeysHandler::invalidKeyByHash: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were changed");
    }
}

PublicKey::Shared ContractorKeysHandler::keyByNumber(
    const TrustLineID trustLineID,
    const KeyNumber number)
{
    string query = "SELECT public_key FROM " + mTableName
                   + " WHERE trust_line_id = ? AND number = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyByNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyByNumber: "
                          "Bad binding of trustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, number);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyByNumber: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto result = make_shared<PublicKey>((byte*)sqlite3_column_blob(stmt, 0));
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("ContractorKeysHandler::keyByNumber: "
                            "There are now records with requested number");
    }
}

PublicKey::Shared ContractorKeysHandler::keyByHash(
    const TrustLineID trustLineID,
    const KeyHash::Shared keyHash)
{
    string query = "SELECT public_key FROM " + mTableName
                   + " WHERE trust_line_id = ? AND hash = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyByHash: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyByHash: "
                          "Bad binding of trustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, keyHash->data(), (int)KeyHash::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyByHash: "
                          "Bad binding of Hash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto result = make_shared<PublicKey>((byte*)sqlite3_column_blob(stmt, 0));
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("ContractorKeysHandler::keyByHash: "
                                "There are now records with requested hash");
    }
}

const KeyHash::Shared ContractorKeysHandler::keyHashByNumber(
    const TrustLineID trustLineID,
    const KeyNumber number)
{
    string query = "SELECT hash FROM " + mTableName
                   + " WHERE trust_line_id = ? AND number = ? AND is_valid = 1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyHashByNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyHashByNumber: "
                          "Bad binding of trustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, number);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyHashByNumber: "
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
        throw NotFoundError("ContractorKeysHandler::keyHashByNumber: "
                                "There are now records with requested number");
    }
}

KeysCount ContractorKeysHandler::availableKeysCnt(
    const TrustLineID trustLineID)
{
    string queryCount = "SELECT count(*) FROM " + mTableName + " WHERE trust_line_id = ? AND is_valid = 1";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::availableKeysCnt: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::availableKeysCnt: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (KeysCount)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return rowCount;
}

void ContractorKeysHandler::removeUnusedKeys(
    const TrustLineID trustLineID)
{
    string queryCount = "DELETE FROM " + mTableName + " WHERE trust_line_id = ? AND is_valid = 1";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::removeUnusedKeys: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::removeUnusedKeys: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("ContractorKeysHandler::removeUnusedKeys: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

vector<PublicKey::Shared> ContractorKeysHandler::publicKeysBySetNumber(
    const TrustLineID trustLineID,
    const KeyNumber keysSetSequenceNumber) const
{
    vector<PublicKey::Shared> result;
    string queryCount = "SELECT count(*) FROM " + mTableName
            + " WHERE trust_line_id = ? AND keys_set_sequence_number = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::publicKeysBySetNumber: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::publicKeysBySetNumber: "
                          "Bad binding of Trust Line ID in count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, keysSetSequenceNumber);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::publicKeysBySetNumber: "
                          "Bad binding of Keys Set Sequence Number in count query; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    result.reserve(rowCount);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    string query = "SELECT public_key FROM " + mTableName
            + " WHERE trust_line_id = ? AND keys_set_sequence_number = ?";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::publicKeysBySetNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::publicKeysBySetNumber: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, keysSetSequenceNumber);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::publicKeysBySetNumber: "
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

void ContractorKeysHandler::deleteKeysByTrustLineID(
    const TrustLineID trustLineID)
{
    string query = "DELETE FROM " + mTableName + " WHERE trust_line_id = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::deleteKeysByTrustLineID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::deleteKeysByTrustLineID: "
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
        throw IOError("ContractorKeysHandler::deleteKeysByTrustLineID: "
                         "Run query; sqlite error: " + to_string(rc));
    }
}

LoggerStream ContractorKeysHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream ContractorKeysHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string ContractorKeysHandler::logHeader() const
{
    stringstream s;
    s << "[ContractorKeysHandler]";
    return s.str();
}
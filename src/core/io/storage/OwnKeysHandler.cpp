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
                   " (hash INTEGER PRIMARY KEY, "
                   "trust_line_id INTEGER NOT NULL, "
                   "public_key BLOB NOT NULL, "
                   "public_key_bytes_count INTEGER NOT NULL, "
                   "private_key BLOB NOT NULL, "
                   "private_key_bytes_count INTEGER NOT NULL, "
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
    const CryptoKey &publicKey,
    const CryptoKey &privateKey,
    uint32_t number)
{
    string query = "INSERT INTO " + mTableName +
                   "(hash, trust_line_id, public_key, public_key_bytes_count, "
                           "private_key, private_key_bytes_count, number) "
                           "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, publicKey.hash());
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Hash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, publicKey.key(),
                           (int)publicKey.keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Public Key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 4, (int)publicKey.keySize());
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Public Key Bytes Count; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 5, privateKey.key(),
                           (int)privateKey.keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Private Key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 6, (int)privateKey.keySize());
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Private Key Bytes Count; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 7, number);
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

pair<uint32_t, CryptoKey> OwnKeysHandler::nextAvailableKey(
    const TrustLineID trustLineID)
{
    string query = "SELECT private_key, private_key_bytes_count, number FROM " + mTableName
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
        auto keyBytesCount = (size_t)sqlite3_column_int(stmt, 1);
        CryptoKey result;
        result.deserialize(
            keyBytesCount,
            (byte*)sqlite3_column_blob(stmt, 0));
        auto number = (uint32_t)sqlite3_column_int(stmt, 2);
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return make_pair(
            number,
            result);
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("OwnKeysHandler::nextAvailableKey: "
                                "There are now records with requested trust line id");
    }
}

void OwnKeysHandler::invalidKey(
    const TrustLineID trustLineID,
    uint32_t number)
{
    string query = "UPDATE " + mTableName + " SET is_valid = 0 WHERE trust_line_id = ? AND number = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::invalidKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::nextAvailableKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, number);
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
}

vector<pair<uint32_t, CryptoKey>> OwnKeysHandler::allAvailablePublicKeys(
    const TrustLineID trustLineID)
{
    info() << "allAvailablePublicKeys";
    string queryCount = "SELECT count(*) FROM " + mTableName + " WHERE trust_line_id = ? AND is_valid = 1";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::allPublicKeys: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::allPublicKeys: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<pair<uint32_t, CryptoKey>> result;
    result.reserve(rowCount);

    string query = "SELECT public_key, public_key_bytes_count, number FROM "
                   + mTableName + " WHERE trust_line_id = ? AND is_valid = 1";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::allPublicKeys:: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::allPublicKeys:: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        auto publicKeyBytesCount = (size_t)sqlite3_column_int(stmt, 1);
        auto number = (uint32_t)sqlite3_column_int(stmt, 2);
        result.emplace_back(
            number,
            CryptoKey(
                (byte*)sqlite3_column_blob(stmt, 0),
                publicKeyBytesCount));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

uint32_t OwnKeysHandler::availableKeysCnt(
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
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return rowCount;
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
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
                   " (hash INTEGER PRIMARY KEY, "
                   "trust_line_id INTEGER NOT NULL, "
                   "public_key BLOB NOT NULL, "
                   "public_key_bytes_count INT NOT NULL, "
                   "number INTEGER NOT NULL, "
                   "is_valid INTEGER NOT NULL DEFAULT 1, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
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
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
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
    const CryptoKey &publicKey,
    uint32_t number)
{
    string query = "INSERT INTO " + mTableName +
                   "(hash, trust_line_id, public_key, public_key_bytes_count, "
                   "number) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, publicKey.hash());
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of Hash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, publicKey.key(),
                       (int)publicKey.keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of Public Key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 4, (int)publicKey.keySize());
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Bad binding of Public Key Bytes Count; sqlite error: " + to_string(rc));
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
        info() << "prepare inserting or replacing is completed successfully";
#endif
    } else {
        throw IOError("ContractorKeysHandler::saveKey: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

CryptoKey ContractorKeysHandler::keyByNumber(
    uint32_t number)
{
    string query = "SELECT public_key, public_key_bytes_count FROM " + mTableName + " WHERE number = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyByNumber: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, number);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorKeysHandler::keyByNumber: "
                          "Bad binding of Number; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto keyBytesCount = (size_t)sqlite3_column_int(stmt, 1);
        CryptoKey result;
        result.deserialize(
            keyBytesCount,
            (byte*)sqlite3_column_blob(stmt, 0));
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

uint32_t ContractorKeysHandler::availableKeysCnt(
    const TrustLineID trustLineID)
{
    string queryCount = "SELECT count(*) FROM " + mTableName + " WHERE trust_line_id = ? AND is_valid = 1";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, 0);
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
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return rowCount;
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
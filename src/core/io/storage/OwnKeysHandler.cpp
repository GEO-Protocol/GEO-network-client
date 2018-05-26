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
                   "(hash INTEGER PRIMARY KEY, "
                   "trust_line_id INTEGER NOT NULL, "
                   "public_key BLOB NOT NULL, "
                   "public_key_bytes_count INT NOT NULL, "
                   "private_key BLOB NOT NULL, "
                   "private_key_bytes_count INT NOT NULL, "
                   "number INTEGER NOT NULL, "
                   "is_valid INTEGER NOT NULL DEFAULT 1, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id));";
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

//    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
//            + "_hash_idx on " + mTableName + "(hash);";
//    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
//    if (rc != SQLITE_OK) {
//        throw IOError("OwnKeysHandler::creating  index for Hash: "
//                          "Bad query; sqlite error: " + to_string(rc));
//    }
//    rc = sqlite3_step(stmt);
//    if (rc == SQLITE_DONE) {
//    } else {
//        throw IOError("OwnKeysHandler::creating index for Hash: "
//                          "Run query; sqlite error: " + to_string(rc));
//    }

//    query = "CREATE INDEX IF NOT EXISTS " + mTableName
//            + "_trust_line_id_idx on " + mTableName + "(trust_line_id);";
//    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
//    if (rc != SQLITE_OK) {
//        throw IOError("OwnKeysHandler::creating  index for Trust Line ID: "
//                          "Bad query; sqlite error: " + to_string(rc));
//    }
//    rc = sqlite3_step(stmt);
//    if (rc == SQLITE_DONE) {
//    } else {
//        throw IOError("OwnKeysHandler::creating index for Trust Line ID: "
//                          "Run query; sqlite error: " + to_string(rc));
//    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void OwnKeysHandler::saveKey(
    CryptoKeyRecord::Shared cryptoKeyRecord,
    const TrustLineID trustLineID)
{
    string query = "INSERT OR REPLACE INTO " + mTableName +
                   "(hash, trust_line_id, public_key, public_key_bytes_count, "
                   "private_key, private_key_bytes_count, number) "
                   "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, cryptoKeyRecord->publicKeyHash());
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Hash; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, cryptoKeyRecord->publicKey().first.get(),
                           (int)cryptoKeyRecord->publicKey().second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Public Key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 4, (int)cryptoKeyRecord->publicKey().second);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Public Key Bytes Count; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 5, cryptoKeyRecord->privateKey().first.get(),
                           (int)cryptoKeyRecord->privateKey().second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Private Key; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 6, (int)cryptoKeyRecord->privateKey().second);
    if (rc != SQLITE_OK) {
        throw IOError("OwnKeysHandler::saveKey: "
                          "Bad binding of Private Key Bytes Count; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 7, cryptoKeyRecord->number());
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
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
                   "private_key BLOB NOT NULL, "
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
#include "AuditHandler.h"

AuditHandler::AuditHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(id INTEGER PRIMARY KEY, "
                   "trust_line_id INTEGER NOT NULL, "
                   "our_sign_hash INTEGER NOT NULL, "
                   "contractor_sign BLOB NOT NULL, "
                   "contractor_sign_bytes_count INT NOT NULL, "
                   "balance BLOB NOT NULL, "
                   "outgoing_amount BLOB NOT NULL, "
                   "incoming_amount BLOB NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("AuditHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

//    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
//            + "_id_idx on " + mTableName + "(id);";
//    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
//    if (rc != SQLITE_OK) {
//        throw IOError("AuditHandler::creating  index for ID: "
//                          "Bad query; sqlite error: " + to_string(rc));
//    }
//    rc = sqlite3_step(stmt);
//    if (rc == SQLITE_DONE) {
//    } else {
//        throw IOError("AuditHandler::creating index for ID: "
//                          "Run query; sqlite error: " + to_string(rc));
//    }

//    query = "CREATE INDEX IF NOT EXISTS " + mTableName
//            + "_trust_line_id_idx on " + mTableName + "(trust_line_id);";
//    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
//    if (rc != SQLITE_OK) {
//        throw IOError("AuditHandler::creating  index for Trust Line ID: "
//                          "Bad query; sqlite error: " + to_string(rc));
//    }
//    rc = sqlite3_step(stmt);
//    if (rc == SQLITE_DONE) {
//    } else {
//        throw IOError("AuditHandler::creating index for Trust Line ID: "
//                          "Run query; sqlite error: " + to_string(rc));
//    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

LoggerStream AuditHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream AuditHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string AuditHandler::logHeader() const
{
    stringstream s;
    s << "[AuditHandler]";
    return s.str();
}
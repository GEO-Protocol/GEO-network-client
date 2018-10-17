#include "AuditRulesHandler.h"

AuditRulesHandler::AuditRulesHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(trust_line_id INTEGER NOT NULL, "
                   "rule_id INTEGER NOT NULL, "
                   "parameters BLOB, "
                   "FOREIGN KEY(trust_line_id) REFERENCES trust_lines(id) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AuditRulesHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("AuditRulesHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_trust_line_id_idx on " + mTableName + "(trust_line_id);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AuditRulesHandler::creating index for TrustLineID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("AuditRulesHandler::creating index for TrustLineID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void AuditRulesHandler::saveRule(
    TrustLineID trustLineID,
    BaseAuditRule::AuditRuleType auditRuleType)
{
    string query = "INSERT INTO " + mTableName +
                   "(trust_line_id, rule_id) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AuditHandler::saveRule: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("AuditRulesHandler::saveRule: "
                          "Bad binding of TrustLineID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, auditRuleType);
    if (rc != SQLITE_OK) {
        throw IOError("AuditRulesHandler::saveRule: "
                          "Bad binding of Rule Type; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("AuditRulesHandler::saveRule: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

const BaseAuditRule::AuditRuleType AuditRulesHandler::getRule(
    TrustLineID trustLineID)
{
    string query = "SELECT rule_id FROM " + mTableName
                   + " WHERE trust_line_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AuditRulesHandler::getRule: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, trustLineID);
    if (rc != SQLITE_OK) {
        throw IOError("AuditRulesHandler::getRule: "
                          "Bad binding of Trust Line ID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto ruleId = (BaseAuditRule::AuditRuleType)sqlite3_column_int(stmt, 0);
        return ruleId;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("AuditRulesHandler::getRule: "
                                "There are no records with requested trust line id");
    }
}

LoggerStream AuditRulesHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream AuditRulesHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string AuditRulesHandler::logHeader() const
{
    stringstream s;
    s << "[AuditRulesHandler]";
    return s.str();
}
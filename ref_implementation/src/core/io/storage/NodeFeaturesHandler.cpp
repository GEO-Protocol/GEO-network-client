/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "NodeFeaturesHandler.h"

NodeFeaturesHandler::NodeFeaturesHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger):

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (feature_name TEXT NOT NULL, "
                       "feature_value TEXT, "
                       "CONSTRAINT feature_name_idx UNIQUE (feature_name));";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("NodeFeaturesHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("NodeFeaturesHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void NodeFeaturesHandler::saveRecord(
    const string &featureName,
    const string &featureValue)
{
    string query = "INSERT OR REPLACE INTO " + mTableName +
                   " (feature_name, feature_value) VALUES(?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("NodeFeaturesHandler::saveRecord: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_text(stmt, 1, featureName.c_str(), featureName.size(), 0);
    if (rc != SQLITE_OK) {
        throw IOError("NodeFeaturesHandler::saveRecord: "
                          "Bad binding of featureName; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, featureValue.c_str(), featureValue.size(), 0);
    if (rc != SQLITE_OK) {
        throw IOError("NodeFeaturesHandler::saveRecord: "
                          "Bad binding of featureValue; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        warning() << "NodeFeaturesHandler::saveRecord: Run query; sqlite error: " << rc;
        throw IOError("NodeFeaturesHandler::saveRecord: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

string NodeFeaturesHandler::featureValue(
    const string &featureName)
{
    string query = "SELECT feature_value FROM " + mTableName + " WHERE feature_name = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("NodeFeaturesHandler::featureValue: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_text(stmt, 1, featureName.c_str(), featureName.size(), 0);
    if (rc != SQLITE_OK) {
        throw IOError("NodeFeaturesHandler::featureValue: "
                              "Bad binding of featureName; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto featureValue = sqlite3_column_text(stmt, 0);
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return string(reinterpret_cast<const char*>(featureValue));
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("NodeFeaturesHandler::featureValue: "
                                "There are now records with requested featureName");
    }
}

void NodeFeaturesHandler::deleteRecord(
    const string &featureName)
{
    string query = "DELETE FROM " + mTableName + " WHERE feature_name = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("NodeFeaturesHandler::deleteRecord: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_text(stmt, 1, featureName.c_str(), featureName.size(), 0);
    if (rc != SQLITE_OK) {
        throw IOError("NodeFeaturesHandler::deleteRecord: "
                          "Bad binding of featureName; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare deleting is completed successfully";
#endif
    } else {
        throw IOError("NodeFeaturesHandler::deleteRecord: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

LoggerStream NodeFeaturesHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream NodeFeaturesHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string NodeFeaturesHandler::logHeader() const
{
    stringstream s;
    s << "[NodeFeaturesHandler]";
    return s.str();
}

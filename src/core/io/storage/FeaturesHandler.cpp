#include "FeaturesHandler.h"

FeaturesHandler::FeaturesHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(feature_name STRING NOT NULL, "
                   "feature_length INTEGER NOT NULL, "
                   "feature_value STRING NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("FeaturesHandler::creating table: "
                      "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("FeaturesHandler::creating table: "
                      "Run query; sqlite error: " + to_string(rc));
    }
    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_feature_name_idx on " + mTableName + " (feature_name);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("FeaturesHandler::creating index for feature name: "
                      "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("FeaturesHandler::creating index for feature name: "
                      "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void FeaturesHandler::saveFeature(
    const string& featureName,
    const string& featureValue)
{
    string query = "INSERT OR REPLACE INTO " + mTableName +
                   "(feature_name, feature_length, feature_value) "
                   "VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("FeaturesHandler::saveFeature: "
                      "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_text(stmt, 1, featureName.c_str(), (int)featureName.size(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("FeaturesHandler::saveFeature: "
                      "Bad binding of feature name; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, (int)featureValue.size());
    if (rc != SQLITE_OK) {
        throw IOError("FeaturesHandler::saveFeature: "
                      "Bad binding of feature size; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_text(stmt, 3, featureValue.c_str(), (int)featureValue.size(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("FeaturesHandler::saveFeature: "
                      "Bad binding of feature value; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("FeaturesHandler::saveFeature: "
                      "Run query; sqlite error: " + to_string(rc));
    }
}

string FeaturesHandler::getFeature(
    const string& featureName)
{
    sqlite3_stmt *stmt;
    string query = "SELECT feature_length, feature_value FROM "
                   + mTableName + " WHERE feature_name = ?";
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("FeaturesHandler::getFeature: "
                      "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_text(stmt, 1, featureName.c_str(), (int)featureName.size(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("FeaturesHandler::getFeature: "
                      "Bad binding of feature name; sqlite error: " + to_string(rc));
    }
    if (sqlite3_step(stmt) != SQLITE_ROW ) {
        throw NotFoundError("FeaturesHandler::getFeature: "
                         "There is no feature: " + featureName);
    }

    auto featureSize = (size_t)sqlite3_column_int(stmt, 0);
    auto featureValueBytes = (byte*)sqlite3_column_blob(stmt, 1);
    string result( reinterpret_cast<char const*>(featureValueBytes), featureSize) ;
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream FeaturesHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream FeaturesHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string FeaturesHandler::logHeader() const
{
    stringstream s;
    s << "[FeaturesHandler]";
    return s.str();
}
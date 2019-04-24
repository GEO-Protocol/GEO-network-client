#include "ContractorsHandler.h"

ContractorsHandler::ContractorsHandler(
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
                   "id_on_contractor_side INTEGER, "
                   "crypto_key BLOB NOT NULL, "
                   "is_confirmed INTEGER NOT NULL DEFAULT 0);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("ContractorsHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_id_idx on " + mTableName + " (id);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::creating index for ID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("ContractorsHandler::creating index for ID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void ContractorsHandler::saveContractor(
    Contractor::Shared contractor)
{
    string query = "INSERT INTO " + mTableName +
                   " (id, crypto_key) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveContractor: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, contractor->getID());
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveContractor: "
                          "Bad binding of ID; sqlite error: " + to_string(rc));
    }

    auto cryptoKey = contractor->cryptoKey();
    vector<byte> cryptoKeyBlob;
    if(cryptoKey)
        cryptoKey->serialize(cryptoKeyBlob);
    rc = sqlite3_bind_blob(stmt, 2, &cryptoKeyBlob[0], cryptoKeyBlob.size(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveContractor: "
                          "Bad binding of cryptoKey; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("ContractorsHandler::saveContractor: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void ContractorsHandler::saveContractorFull(
    Contractor::Shared contractor)
{
    string query = "INSERT INTO " + mTableName +
                   "(id, id_on_contractor_side, crypto_key, is_confirmed) "
                   "VALUES (?, ?, ?, 1);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveContractorFull: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, contractor->getID());
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveContractorFull: "
                          "Bad binding of ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, contractor->ownIdOnContractorSide());
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveContractorFull: "
                          "Bad binding of ID on contractor side; sqlite error: " + to_string(rc));
    }
    auto cryptoKey = contractor->cryptoKey();
    vector<byte> cryptoKeyBlob;
    if(cryptoKey)
        cryptoKey->serialize(cryptoKeyBlob);
    rc = sqlite3_bind_blob(stmt, 3, &cryptoKeyBlob[0], cryptoKeyBlob.size(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveContractorFull: "
                          "Bad binding of cryptoKey; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("ContractorsHandler::saveContractorFull: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void ContractorsHandler::saveConfirmationInfo(
    Contractor::Shared contractor)
{
    string query = "UPDATE " + mTableName +
                   " SET id_on_contractor_side = ?, crypto_key = ?, is_confirmed = 1 WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveConfirmationInfo: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, contractor->ownIdOnContractorSide());
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveConfirmationInfo: "
                          "Bad binding of ID on contractor side; sqlite error: " + to_string(rc));
    }
    auto cryptoKey = contractor->cryptoKey();
    vector<byte> cryptoKeyBlob;
    if(cryptoKey) {
        cryptoKey->serialize(cryptoKeyBlob);
    }
    rc = sqlite3_bind_blob(stmt, 2, &cryptoKeyBlob[0], cryptoKeyBlob.size(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveConfirmationInfo: "
                          "Bad binding of cryptoKey; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, contractor->getID());
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::saveConfirmationInfo: "
                          "Bad binding of ID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "updateing is completed successfully";
#endif
    } else {
        throw IOError("ContractorsHandler::saveConfirmationInfo: "
                      "Run query; sqlite error: " + to_string(rc));
    }

    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were modified");
    }
}

vector<Contractor::Shared> ContractorsHandler::allContractors()
{
    string queryCount = "SELECT count(*) FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::allContractors: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<Contractor::Shared> result;
    result.reserve(rowCount);

    string query = "SELECT id, id_on_contractor_side, crypto_key, is_confirmed FROM " + mTableName;
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::allContractors: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        auto id = (ContractorID)sqlite3_column_int(stmt, 0);
        auto idOnContractorSide = (ContractorID)sqlite3_column_int(stmt, 1);

        vector<uint8_t> cryptoKeyBlob(sqlite3_column_bytes(stmt, 2));
        memcpy(
            &cryptoKeyBlob[0],
            sqlite3_column_blob(stmt, 2),
            cryptoKeyBlob.size());
        MsgEncryptor::KeyTrio::Shared cryptoKey =
            make_shared<MsgEncryptor::KeyTrio>(cryptoKeyBlob);

        auto isChannelConfirmed = sqlite3_column_int(stmt, 3);

        try {
            result.push_back(
                make_shared<Contractor>(
                    id,
                    idOnContractorSide,
                    cryptoKey,
                    isChannelConfirmed == 1));
        } catch (...) {
            throw Exception("ContractorsHandler::allContractors. "
                                "Unable to create contractor instance from DB.");
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<TrustLineID> ContractorsHandler::allIDs()
{
    string queryCount = "SELECT count(*) FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::allIDs: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<ContractorID> result;
    result.reserve(rowCount);

    string query = "SELECT id FROM " + mTableName;
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw IOError("ContractorsHandler::allIDs: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result.push_back(
            (ContractorID)sqlite3_column_int(stmt, 0));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream ContractorsHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream ContractorsHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string ContractorsHandler::logHeader() const
{
    stringstream s;
    s << "[ContractorsHandler]";
    return s.str();
}
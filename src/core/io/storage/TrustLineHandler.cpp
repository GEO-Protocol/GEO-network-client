#include "TrustLineHandler.h"

TrustLineHandler::TrustLineHandler(
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
                   "state INTEGER NOT NULL, "
                   "contractor_id INTEGER NOT NULL, "
                   "equivalent INTEGER NOT NULL, "
                   "is_contractor_gateway INTEGER NOT NULL DEFAULT 0, "
                   "FOREIGN KEY(contractor_id) REFERENCES contractors(id) ON DELETE CASCADE ON UPDATE CASCADE);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("TrustLineHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_id_idx on " + mTableName + "(id);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::creating index for ID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("TrustLineHandler::creating index for ID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_equivalent_idx on " + mTableName + "(equivalent);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::creating index for Equivalent: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("TrustLineHandler::creating index for Equivalent: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_contractor_id_equivalent_idx on " + mTableName + "(contractor_id, equivalent);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::creating unique index for ContractorID and Equivalent: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("TrustLineHandler::creating unique index for ContractorID and Equivalent: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

vector<TrustLine::Shared> TrustLineHandler::allTrustLinesByEquivalent(
    const SerializedEquivalent equivalent)
{
    string queryCount = "SELECT count(*) FROM " + mTableName + " WHERE equivalent = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::allTrustLinesByEquivalent: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, equivalent);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::allTrustLinesByEquivalent: "
                          "Bad binding of Equivalent; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<TrustLine::Shared> result;
    result.reserve(rowCount);

    string query = "SELECT id, state, contractor_id, is_contractor_gateway FROM "
                   + mTableName + " WHERE equivalent = ?";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::allTrustLinesByEquivalent: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 1, equivalent);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::allTrustLinesByEquivalent: "
                          "Bad binding of Equivalent; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        auto id = (TrustLineID)sqlite3_column_int(stmt, 0);

        auto state = (TrustLine::TrustLineState)sqlite3_column_int(stmt, 1);

        auto contractorID = (ContractorID)sqlite3_column_int(stmt, 2);

        int32_t isContractorGateway = sqlite3_column_int(stmt, 3);

        try {
            result.push_back(
                make_shared<TrustLine>(
                    id,
                    contractorID,
                    isContractorGateway != 0,
                    state));
        } catch (...) {
            throw Exception("TrustLinesManager::allTrustLinesByEquivalent. "
                                "Unable to create trust line instance from DB.");
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

void TrustLineHandler::deleteTrustLine(
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent)
{
    string query = "DELETE FROM " + mTableName + " WHERE contractor = ? AND equivalent = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::deleteTrustLine: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, contractorUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::deleteTrustLine: "
                          "Bad binding of Contractor; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, equivalent);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::deleteTrustLine: "
                          "Bad binding of Equivalent; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "deleting is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::deleteTrustLine: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void TrustLineHandler::saveTrustLine(
    TrustLine::Shared trustLine,
    const SerializedEquivalent equivalent)
{
    string query = "INSERT INTO " + mTableName +
                   "(id, state, contractor_id, equivalent, is_contractor_gateway) "
                   "VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::saveTrustLine: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, trustLine->trustLineID());
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::saveTrustLine: "
                          "Bad binding of ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, (int)trustLine->state());
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::saveTrustLine: "
                          "Bad binding of State; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, trustLine->contractorID());
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::saveTrustLine: "
                          "Bad binding of ContractorID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 4, equivalent);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::saveTrustLine: "
                          "Bad binding of Equivalent; sqlite error: " + to_string(rc));
    }
    int32_t isContractorGateway = trustLine->isContractorGateway();
    rc = sqlite3_bind_int(stmt, 5, isContractorGateway);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::saveTrustLine: "
                          "Bad binding of IsContractorGateway; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::saveTrustLine: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

void TrustLineHandler::updateTrustLineState(
    TrustLine::Shared trustLine,
    const SerializedEquivalent equivalent)
{
    string query = "UPDATE " + mTableName +
                   " SET state = ? "
                   "WHERE id = ? AND equivalent = ? AND contractor_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineState: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, (int)trustLine->state());
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineState: "
                          "Bad binding of State; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLine->trustLineID());
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineState: "
                          "Bad binding of ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, equivalent);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineState: "
                          "Bad binding of Equivalent; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 4, trustLine->contractorID());
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineState: "
                          "Bad binding of ContractorID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare updating is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::updateTrustLineState: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were modified");
    }
}

void TrustLineHandler::updateTrustLineIsContractorGateway(
    TrustLine::Shared trustLine,
    const SerializedEquivalent equivalent)
{
    string query = "UPDATE " + mTableName +
                   " SET is_contractor_gateway = ? "
                   "WHERE id = ? AND equivalent = ? AND contractor_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineIsContractorGateway: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    int32_t isContractorGateway = trustLine->isContractorGateway();
    rc = sqlite3_bind_int(stmt, 1, isContractorGateway);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineIsContractorGateway: "
                          "Bad binding of IsContractorGateway; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 2, trustLine->trustLineID());
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineIsContractorGateway: "
                          "Bad binding of ID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 3, equivalent);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineIsContractorGateway: "
                          "Bad binding of Equivalent; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_int(stmt, 4, trustLine->contractorID());
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::updateTrustLineIsContractorGateway: "
                          "Bad binding of ContractorID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare updating is completed successfully";
#endif
    } else {
        throw IOError("TrustLineHandler::updateTrustLineIsContractorGateway: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    if (sqlite3_changes(mDataBase) == 0) {
        throw ValueError("No data were modified");
    }
}

vector<SerializedEquivalent> TrustLineHandler::equivalents()
{
    string query = "SELECT DISTINCT equivalent FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::equivalents: "
                          "Bad query; sqlite error: " + to_string(rc));
    }

    vector<SerializedEquivalent> result;
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        auto equivalent = (SerializedEquivalent)sqlite3_column_int(stmt, 0);
        result.push_back(equivalent);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<TrustLineID> TrustLineHandler::allIDs()
{
    string queryCount = "SELECT count(*) FROM " + mTableName;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, queryCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::allIDs: "
                          "Bad count query; sqlite error: " + to_string(rc));
    }
    sqlite3_step(stmt);
    auto rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<TrustLineID> result;
    result.reserve(rowCount);

    string query = "SELECT id FROM " + mTableName;
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::allIDs: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result.push_back(
            (TrustLineID)sqlite3_column_int(stmt, 0));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream TrustLineHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream TrustLineHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string TrustLineHandler::logHeader() const
{
    stringstream s;
    s << "[TrustLineHandler]";
    return s.str();
}

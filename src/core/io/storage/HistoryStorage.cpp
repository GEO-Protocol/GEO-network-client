#include "HistoryStorage.h"

HistoryStorage::HistoryStorage(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger *logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   "(operation_uuid BLOB NOT NULL, "
                       "operation_timestamp INTEGER NOT NULL, "
                       "record_type INTEGER NOT NULL, "
                       "record_body BLOB NOT NULL, "
                       "record_body_bytes_count INT NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::creating table: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::creating table: Run query; sqlite error: " + rc);
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void HistoryStorage::commit() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call commit, but trunsaction wasn't started";
#endif
        return;
    }
    string query = "COMMIT TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::commit: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("HistoryStorage::commit: Run query; sqlite error: " + rc);
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction commit";
#endif
    isTransactionBegin = false;
}

void HistoryStorage::rollBack() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call rollBack, but trunsaction wasn't started";
#endif
        return;
    }
    string query = "ROLLBACK TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::rollback: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("HistoryStorage::rollback: Run query; sqlite error: " + rc);
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "rollBack done";
#endif
    isTransactionBegin = false;
}

void HistoryStorage::prepareInserted() {

    if (isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call prepareInsertred, but previous transaction isn't finished";
#endif
        return;
    }
    string query = "BEGIN TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::prepareInserted: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("HistoryStorage::prepareInserted: Run query; sqlite error: " + rc);
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction begin";
#endif
    isTransactionBegin = true;
}

void HistoryStorage::saveRecord(Record::Shared record) {

    if (!isTransactionBegin) {
        prepareInserted();
    }
    string query = "INSERT INTO " + mTableName
                   + "(operation_uuid, operation_timestamp, record_type, record_body, record_body_bytes_count) "
                                                             "VALUES(?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_bind_blob(stmt, 1, record->operationUUID().data, Record::kOperationUUIDBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of OperationUUID; sqlite error: " + rc);
    }
    GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(utc_now());
    rc = sqlite3_bind_int64(stmt, 2, timestamp);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of Timestamp; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 3, record->recordType());
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of RecordType; sqlite error: " + rc);
    }
    auto serializedRecordAndSize = record->serializeToBytes();
    rc = sqlite3_bind_blob(stmt, 4, serializedRecordAndSize.first.get(), (int)serializedRecordAndSize.second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of RecordBody; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 5, (int)serializedRecordAndSize.second);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of RecordBody bytes count; sqlite error: " + rc);
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("HistoryStorage::insert: "
                          "Run query; sqlite error: " + rc);
    }
}

vector<pair<PaymentRecord::Shared, DateTime>> HistoryStorage::allPaymentRecords(
    size_t recordsCount,
    size_t fromRecord) {

    vector<pair<PaymentRecord::Shared, DateTime>> result;
    string query = "SELECT operation_uuid, record_body, record_body_bytes_count, operation_timestamp FROM "
                   + mTableName + " WHERE record_type = ? LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allPaymentRecords: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 1, Record::RecordType::PaymentRecordType);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of RecordType; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 2, (int)recordsCount);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of recordsCount; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 3, (int)fromRecord);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of fromRecord; sqlite error: " + rc);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        uuids::uuid operationUUID;
        memcpy(
            operationUUID.data,
            sqlite3_column_blob(stmt, 0),
            Record::kOperationUUIDBytesSize);
        size_t recordBodyBytesCount = (size_t)sqlite3_column_int(stmt, 2);
        BytesShared recordBody = tryMalloc(recordBodyBytesCount);
        memcpy(
            recordBody.get(),
            sqlite3_column_blob(stmt, 1),
            recordBodyBytesCount);
        GEOEpochTimestamp timestamp = (GEOEpochTimestamp)sqlite3_column_int64(stmt, 3);
        result.push_back(
            make_pair(
                make_shared<PaymentRecord>(
                    operationUUID,
                    recordBody),
                dateTimeFromGEOEpochTimestamp(
                    timestamp)));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<pair<TrustLineRecord::Shared, DateTime>> HistoryStorage::allTrustLineRecords(
    size_t recordsCount,
    size_t fromRecord) {

    vector<pair<TrustLineRecord::Shared, DateTime>> result;
    string query = "SELECT operation_uuid, record_body, record_body_bytes_count, operation_timestamp FROM "
                   + mTableName + " WHERE record_type = ? LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allPaymentRecords: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 1, Record::RecordType::TrustLineRecordType);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of RecordType; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 2, (int)recordsCount);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of recordsCount; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 3, (int)fromRecord);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert: "
                          "Bad binding of fromRecord; sqlite error: " + rc);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        uuids::uuid operationUUID;
        memcpy(
            operationUUID.data,
            sqlite3_column_blob(stmt, 0),
            Record::kOperationUUIDBytesSize);
        size_t recordBodyBytesCount = (size_t)sqlite3_column_int(stmt, 2);
        BytesShared recordBody = tryMalloc(recordBodyBytesCount);
        memcpy(
            recordBody.get(),
            sqlite3_column_blob(stmt, 1),
            recordBodyBytesCount);

        GEOEpochTimestamp timestamp = (GEOEpochTimestamp)sqlite3_column_int64(stmt, 3);
        result.push_back(
            make_pair(
                make_shared<TrustLineRecord>(
                    operationUUID,
                    recordBody),
                dateTimeFromGEOEpochTimestamp(
                    timestamp)));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

LoggerStream HistoryStorage::info() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream HistoryStorage::error() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string HistoryStorage::logHeader() const
{
    stringstream s;
    s << "[HistoryStorage]";
    return s.str();
}

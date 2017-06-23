#include "HistoryStorage.h"

HistoryStorage::HistoryStorage(
    sqlite3 *dbConnection,
    const string &mainTableName,
    const string &additionalTableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mMainTableName(mainTableName),
    mAdditionalTableName(additionalTableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    // creating main table
    string query = "CREATE TABLE IF NOT EXISTS " + mMainTableName +
                   "(operation_uuid BLOB NOT NULL, "
                       "operation_timestamp INTEGER NOT NULL, "
                       "record_type INTEGER NOT NULL, "
                       "record_body BLOB NOT NULL, "
                       "record_body_bytes_count INT NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::creating main table: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::creating main table: Run query; sqlite error: " + rc);
    }
    query = "CREATE INDEX IF NOT EXISTS " + mMainTableName
            + "_operation_uuid_idx on " + mMainTableName + "(operation_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::main creating index for operation_uuid: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::main creating index for operation_uuid: "
                          "Run query; sqlite error: " + rc);
    }
    query = "CREATE INDEX IF NOT EXISTS " + mMainTableName
            + "_operation_timestamp_idx on " + mMainTableName + "(operation_timestamp);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::main creating index for operation_timestamp: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::main creating index for operation_timestamp: "
                          "Run query; sqlite error: " + rc);
    }
    query = "CREATE INDEX IF NOT EXISTS " + mMainTableName
            + "_record_type_idx on " + mMainTableName + "(record_type);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::main creating index for record_type: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::main creating index for record_type: "
                          "Run query; sqlite error: " + rc);
    }

    // creating payments additional table
    query = "CREATE TABLE IF NOT EXISTS " + mAdditionalTableName +
                   "(operation_uuid BLOB NOT NULL, "
                       "operation_timestamp INTEGER NOT NULL, "
                       "record_type INTEGER NOT NULL, "
                       "record_body BLOB NOT NULL, "
                       "record_body_bytes_count INT NOT NULL);";
    rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::creating additional table: Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::creating additional table: Run query; sqlite error: " + rc);
    }
    query = "CREATE INDEX IF NOT EXISTS " + mAdditionalTableName
            + "_operation_uuid_idx on " + mAdditionalTableName + "(operation_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::additional creating index for operation_uuid: "
                              "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::additional creating index for operation_uuid: "
                              "Run query; sqlite error: " + rc);
    }
    query = "CREATE INDEX IF NOT EXISTS " + mAdditionalTableName
            + "_operation_timestamp_idx on " + mAdditionalTableName + "(operation_timestamp);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::additional creating index for operation_timestamp: "
                              "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::additional creating index for operation_timestamp: "
                              "Run query; sqlite error: " + rc);
    }
    query = "CREATE INDEX IF NOT EXISTS " + mAdditionalTableName
            + "_record_type_idx on " + mAdditionalTableName + "(record_type);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::additional creating index for record_type: "
                              "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("HistoryStorage::additional creating index for record_type: "
                              "Run query; sqlite error: " + rc);
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void HistoryStorage::saveTrustLineRecord(
    TrustLineRecord::Shared record)
{
    string query = "INSERT INTO " + mMainTableName
                   + "(operation_uuid, operation_timestamp, record_type, record_body, record_body_bytes_count) "
                       "VALUES(?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert trustline: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_bind_blob(stmt, 1, record->operationUUID().data, Record::kOperationUUIDBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert trustline: "
                          "Bad binding of OperationUUID; sqlite error: " + rc);
    }
    GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(record->timestamp());
    rc = sqlite3_bind_int64(stmt, 2, timestamp);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert trustline: "
                          "Bad binding of Timestamp; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 3, record->recordType());
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert trustline: "
                          "Bad binding of RecordType; sqlite error: " + rc);
    }
    auto serializedTrustLineRecordAndSize = serializedTrustLineRecordBody(
        record);
    rc = sqlite3_bind_blob(stmt, 4, serializedTrustLineRecordAndSize.first.get(),
                           (int) serializedTrustLineRecordAndSize.second,
                           SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert trustline: "
                          "Bad binding of RecordBody; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 5, (int) serializedTrustLineRecordAndSize.second);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert trustline: "
                          "Bad binding of RecordBody bytes count; sqlite error: " + rc);
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting of trustline is completed successfully";
#endif
    } else {
        throw IOError("HistoryStorage::insert trustline: "
                          "Run query; sqlite error: " + rc);
    }
}

void HistoryStorage::savePaymentRecord(
    PaymentRecord::Shared record)
{
    switch (record->paymentOperationType()) {
        case PaymentRecord::IncomingPaymentType:
        case PaymentRecord::OutgoingPaymentType:
            savePaymentMainRecord(record);
            break;
        case PaymentRecord::IntermediatePaymentType:
        case PaymentRecord::CycleCloserType:
        case PaymentRecord::CyclerCloserIntermediateType:
            savePaymentAdditionalRecord(record);
            break;
        default:
            throw ValueError("HistoryStorage::savePaymentRecord: "
                                     "invalid payment operation type");
    }
}

void HistoryStorage::savePaymentMainRecord(
        PaymentRecord::Shared record)
{
    string query = "INSERT INTO " + mMainTableName
                   + "(operation_uuid, operation_timestamp, record_type, record_body, record_body_bytes_count) "
                       "VALUES(?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert main payment: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_bind_blob(stmt, 1, record->operationUUID().data, Record::kOperationUUIDBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert main payment: "
                          "Bad binding of OperationUUID; sqlite error: " + rc);
    }
    GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(record->timestamp());
    rc = sqlite3_bind_int64(stmt, 2, timestamp);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert main payment: "
                          "Bad binding of Timestamp; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 3, record->recordType());
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert main payment: "
                          "Bad binding of RecordType; sqlite error: " + rc);
    }

    auto serializedPymentRecordAndSize = serializedPaymentRecordBody(
        record);
    rc = sqlite3_bind_blob(stmt, 4, serializedPymentRecordAndSize.first.get(),
                           (int) serializedPymentRecordAndSize.second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert main payment: "
                          "Bad binding of RecordBody; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 5, (int) serializedPymentRecordAndSize.second);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert main payment: "
                          "Bad binding of RecordBody bytes count; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting of main payment is completed successfully";
#endif
    } else {
        throw IOError("HistoryStorage::insert main payment: "
                          "Run query; sqlite error: " + rc);
    }
}

void HistoryStorage::savePaymentAdditionalRecord(
        PaymentRecord::Shared record)
{
    string query = "INSERT INTO " + mAdditionalTableName
                   + "(operation_uuid, operation_timestamp, record_type, record_body, record_body_bytes_count) "
                           "VALUES(?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert additional payment: "
                              "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_bind_blob(stmt, 1, record->operationUUID().data, Record::kOperationUUIDBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert additional payment: "
                              "Bad binding of OperationUUID; sqlite error: " + rc);
    }
    GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(record->timestamp());
    rc = sqlite3_bind_int64(stmt, 2, timestamp);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert additional payment: "
                              "Bad binding of Timestamp; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 3, record->recordType());
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert additional payment: "
                              "Bad binding of RecordType; sqlite error: " + rc);
    }

    auto serializedPymentRecordAndSize = serializedPaymentAdditionalRecordBody(
        record);
    rc = sqlite3_bind_blob(stmt, 4, serializedPymentRecordAndSize.first.get(),
                           (int) serializedPymentRecordAndSize.second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert additional payment: "
                              "Bad binding of RecordBody; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 5, (int) serializedPymentRecordAndSize.second);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::insert additional payment: "
                              "Bad binding of RecordBody bytes count; sqlite error: " + rc);
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting of additional payment is completed successfully";
#endif
    } else {
        throw IOError("HistoryStorage::insert additional payment: "
                              "Run query; sqlite error: " + rc);
    }
}

vector<TrustLineRecord::Shared> HistoryStorage::allTrustLineRecords(
    size_t recordsCount,
    size_t fromRecord,
    DateTime timeFrom,
    bool isTimeFromPresent,
    DateTime timeTo,
    bool isTimeToPresent)
{
    vector<TrustLineRecord::Shared> result;
    string query = "SELECT operation_uuid, operation_timestamp, record_body, record_body_bytes_count FROM "
                   + mMainTableName + " WHERE record_type = ? ";
    if (isTimeFromPresent) {
        query += " AND operation_timestamp >= ? ";
    }
    if (isTimeToPresent) {
        query += " AND operation_timestamp <= ? ";
    }
    query += " ORDER BY operation_timestamp DESC LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allTrustLineRecords: "
                          "Bad query; sqlite error: " + rc);
    }
    int idxParam = 1;
    rc = sqlite3_bind_int(stmt, idxParam++, Record::RecordType::TrustLineRecordType);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allTrustLineRecords: "
                          "Bad binding of RecordType; sqlite error: " + rc);
    }
    if (isTimeFromPresent) {
        GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(timeFrom);
        rc = sqlite3_bind_int64(stmt, idxParam++, timestamp);
        if (rc != SQLITE_OK) {
            throw IOError("HistoryStorage::allTrustLineRecords: "
                              "Bad binding of TimeFrom; sqlite error: " + rc);
        }
    }
    if (isTimeToPresent) {
        GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(timeTo);
        rc = sqlite3_bind_int64(stmt, idxParam++, timestamp);
        if (rc != SQLITE_OK) {
            throw IOError("HistoryStorage::allTrustLineRecords: "
                              "Bad binding of TimeTo; sqlite error: " + rc);
        }
    }
    rc = sqlite3_bind_int(stmt, idxParam++, (int)recordsCount);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allTrustLineRecords: "
                          "Bad binding of recordsCount; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, idxParam, (int)fromRecord);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allTrustLineRecords: "
                          "Bad binding of fromRecord; sqlite error: " + rc);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result.push_back(
            deserializeTrustLineRecord(
                stmt));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<PaymentRecord::Shared> HistoryStorage::allPaymentRecords(
    size_t recordsCount,
    size_t fromRecord,
    DateTime timeFrom,
    bool isTimeFromPresent,
    DateTime timeTo,
    bool isTimeToPresent)
{
    vector<PaymentRecord::Shared> result;
    string query = "SELECT operation_uuid, operation_timestamp, record_body, record_body_bytes_count FROM "
                   + mMainTableName + " WHERE record_type = ? ";
    if (isTimeFromPresent) {
        query += " AND operation_timestamp >= ? ";
    }
    if (isTimeToPresent) {
        query += " AND operation_timestamp <= ? ";
    }
    query += " ORDER BY operation_timestamp DESC LIMIT ? OFFSET ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allPaymentRecords: "
                          "Bad query; sqlite error: " + rc);
    }
    int idxParam = 1;
    rc = sqlite3_bind_int(stmt, idxParam++, Record::RecordType::PaymentRecordType);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allPaymentRecords: "
                          "Bad binding of RecordType; sqlite error: " + rc);
    }
    if (isTimeFromPresent) {
        GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(timeFrom);
        rc = sqlite3_bind_int64(stmt, idxParam++, timestamp);
        if (rc != SQLITE_OK) {
            throw IOError("HistoryStorage::allPaymentRecords: "
                              "Bad binding of TimeFrom; sqlite error: " + rc);
        }
    }
    if (isTimeToPresent) {
        GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(timeTo);
        rc = sqlite3_bind_int64(stmt, idxParam++, timestamp);
        if (rc != SQLITE_OK) {
            throw IOError("HistoryStorage::allPaymentRecords: "
                              "Bad binding of TimeTo; sqlite error: " + rc);
        }
    }
    rc = sqlite3_bind_int(stmt, idxParam++, (int)recordsCount);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allPaymentRecords: "
                          "Bad binding of recordsCount; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, idxParam, (int)fromRecord);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allPaymentRecords: "
                          "Bad binding of fromRecord; sqlite error: " + rc);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result.push_back(
            deserializePaymentRecord(
                stmt));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

size_t HistoryStorage::countRecordsByType(
    Record::RecordType recordType)
{
    string query = "SELECT count(*) FROM "
                   + mMainTableName + " WHERE record_type = ? ";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::countRecordsByType: "
                          "Bad query; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 1, recordType);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::countRecordsByType: "
                          "Bad binding of RecordType; sqlite error: " + rc);
    }
    sqlite3_step(stmt);
    size_t result = (size_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<PaymentRecord::Shared> HistoryStorage::allPaymentRecords(
    size_t recordsCount,
    size_t fromRecord,
    DateTime timeFrom,
    bool isTimeFromPresent,
    DateTime timeTo,
    bool isTimeToPresent,
    const TrustLineAmount& lowBoundaryAmount,
    bool isLowBoundaryAmountPresent,
    const TrustLineAmount& highBoundaryAmount,
    bool isHighBoundaryAmountPresent)
{
    if (!isLowBoundaryAmountPresent && !isHighBoundaryAmountPresent) {
        return allPaymentRecords(
            recordsCount,
            fromRecord,
            timeFrom,
            isTimeFromPresent,
            timeTo,
            isTimeToPresent);
    }
    vector<PaymentRecord::Shared> result;
    size_t paymentRecordsCount = countRecordsByType(
        Record::RecordType::PaymentRecordType);
    size_t currentOffset = 0;
    size_t countRecordsUnderConditions = 0;
    while (result.size() < recordsCount && currentOffset < paymentRecordsCount) {
        auto paymentRecords = allPaymentRecords(
            kPortionRequestSize,
            currentOffset,
            timeFrom,
            isTimeFromPresent,
            timeTo,
            isTimeToPresent);
        for (auto paymentRecord : paymentRecords) {
            bool recordUnderConditions = true;
            if (isLowBoundaryAmountPresent) {
                recordUnderConditions = recordUnderConditions &&
                    (paymentRecord->amount() >= lowBoundaryAmount);
            }
            if (isHighBoundaryAmountPresent) {
                recordUnderConditions = recordUnderConditions &&
                    (paymentRecord->amount() <= highBoundaryAmount);
            }
            if (recordUnderConditions) {
                countRecordsUnderConditions++;
                if (countRecordsUnderConditions > fromRecord) {
                    result.push_back(paymentRecord);
                }
            }
            if (result.size() >= recordsCount) {
                break;
            }
        }
        currentOffset += kPortionRequestSize;
    }
    return result;
}

vector<PaymentRecord::Shared> HistoryStorage::allPaymentAdditionalRecords()
{
    vector<PaymentRecord::Shared> result;
    string query = "SELECT operation_uuid, operation_timestamp, record_body, record_body_bytes_count FROM "
                   + mAdditionalTableName + " ORDER BY operation_timestamp DESC;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("HistoryStorage::allPaymentAdditionalRecords: "
                              "Bad query; sqlite error: " + rc);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result.push_back(
            deserializePaymentAdditionalRecord(
            stmt));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

pair<BytesShared, size_t> HistoryStorage::serializedTrustLineRecordBody(
    TrustLineRecord::Shared trustLineRecord)
{
    size_t recordBodySize = sizeof(TrustLineRecord::SerializedTrustLineOperationType) + NodeUUID::kBytesSize;
    if (trustLineRecord->trustLineOperationType() != TrustLineRecord::TrustLineOperationType::Closing &&
        trustLineRecord->trustLineOperationType() != TrustLineRecord::TrustLineOperationType::Rejecting) {
        recordBodySize += kTrustLineAmountBytesCount;
    }

    BytesShared bytesBuffer = tryCalloc(
        recordBodySize);
    size_t bytesBufferOffset = 0;

    TrustLineRecord::SerializedTrustLineOperationType operationType
        = (TrustLineRecord::SerializedTrustLineOperationType) trustLineRecord->trustLineOperationType();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &operationType,
        sizeof(TrustLineRecord::SerializedTrustLineOperationType));
    bytesBufferOffset += sizeof(
        TrustLineRecord::SerializedTrustLineOperationType);

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustLineRecord->contractorUUID().data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    if (trustLineRecord->trustLineOperationType() != TrustLineRecord::TrustLineOperationType::Closing &&
        trustLineRecord->trustLineOperationType() != TrustLineRecord::TrustLineOperationType::Rejecting) {
        auto trustAmountBytes = trustLineAmountToBytes(
            trustLineRecord->amount());

        memcpy(
            bytesBuffer.get() + bytesBufferOffset,
            trustAmountBytes.data(),
            kTrustLineAmountBytesCount);
    }
    return make_pair(
        bytesBuffer,
        recordBodySize);
}

pair<BytesShared, size_t> HistoryStorage::serializedPaymentRecordBody(
    PaymentRecord::Shared paymentRecord)
{
    size_t recordBodySize = sizeof(PaymentRecord::SerializedPaymentOperationType)
           + NodeUUID::kBytesSize
           + kTrustLineAmountBytesCount
           + kTrustLineBalanceSerializeBytesCount;

    BytesShared bytesBuffer = tryCalloc(
        recordBodySize);
    size_t bytesBufferOffset = 0;

    PaymentRecord::SerializedPaymentOperationType operationType
        = (PaymentRecord::SerializedPaymentOperationType) paymentRecord->paymentOperationType();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &operationType,
        sizeof(PaymentRecord::SerializedPaymentOperationType));
    bytesBufferOffset += sizeof(
        PaymentRecord::SerializedPaymentOperationType);

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        paymentRecord->contractorUUID().data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    auto trustAmountBytes = trustLineAmountToBytes(
        paymentRecord->amount());
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustAmountBytes.data(),
        kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    auto trustBalanceBytes = trustLineBalanceToBytes(
        paymentRecord->balanceAfterOperation());
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustBalanceBytes.data(),
        kTrustLineBalanceSerializeBytesCount);

    return make_pair(
        bytesBuffer,
        recordBodySize);
}

pair<BytesShared, size_t> HistoryStorage::serializedPaymentAdditionalRecordBody(
        PaymentRecord::Shared paymentRecord)
{
    size_t recordBodySize = sizeof(PaymentRecord::SerializedPaymentOperationType)
                + kTrustLineAmountBytesCount;

    BytesShared bytesBuffer = tryCalloc(
        recordBodySize);
    size_t bytesBufferOffset = 0;

    PaymentRecord::SerializedPaymentOperationType operationType
        = (PaymentRecord::SerializedPaymentOperationType) paymentRecord->paymentOperationType();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &operationType,
        sizeof(PaymentRecord::SerializedPaymentOperationType));
    bytesBufferOffset += sizeof(
            PaymentRecord::SerializedPaymentOperationType);

    auto trustAmountBytes = trustLineAmountToBytes(
        paymentRecord->amount());
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustAmountBytes.data(),
        kTrustLineAmountBytesCount);

    return make_pair(
        bytesBuffer,
        recordBodySize);
}

TrustLineRecord::Shared HistoryStorage::deserializeTrustLineRecord(
    sqlite3_stmt *stmt)
{
    TransactionUUID operationUUID((uint8_t *)sqlite3_column_blob(stmt, 0));
    GEOEpochTimestamp timestamp = (GEOEpochTimestamp)sqlite3_column_int64(stmt, 1);
    size_t recordBodyBytesCount = (size_t)sqlite3_column_int(stmt, 3);
    BytesShared recordBody = tryMalloc(recordBodyBytesCount);
    memcpy(
        recordBody.get(),
        sqlite3_column_blob(stmt, 2),
        recordBodyBytesCount);

    size_t dataBufferOffset = 0;
    TrustLineRecord::SerializedTrustLineOperationType *operationType =
        new (recordBody.get() + dataBufferOffset) TrustLineRecord::SerializedTrustLineOperationType;
    dataBufferOffset += sizeof(
        TrustLineRecord::SerializedTrustLineOperationType);

    NodeUUID contractorUUID(recordBody.get() + dataBufferOffset);
    dataBufferOffset += NodeUUID::kBytesSize;

    TrustLineAmount amount(0);
    if (*operationType != TrustLineRecord::TrustLineOperationType::Closing &&
        *operationType != TrustLineRecord::TrustLineOperationType::Rejecting) {
        vector<byte> amountBytes(
            recordBody.get() + dataBufferOffset,
            recordBody.get() + dataBufferOffset + kTrustLineAmountBytesCount);

        amount = bytesToTrustLineAmount(
            amountBytes);
    }
    return make_shared<TrustLineRecord>(
        operationUUID,
        (TrustLineRecord::TrustLineOperationType) *operationType,
        contractorUUID,
        amount,
        timestamp);
}

PaymentRecord::Shared HistoryStorage::deserializePaymentRecord(
    sqlite3_stmt *stmt)
{
    TransactionUUID operationUUID((uint8_t *)sqlite3_column_blob(stmt, 0));
    GEOEpochTimestamp timestamp = (GEOEpochTimestamp)sqlite3_column_int64(stmt, 1);
    size_t recordBodyBytesCount = (size_t)sqlite3_column_int(stmt, 3);
    BytesShared recordBody = tryMalloc(recordBodyBytesCount);
    memcpy(
        recordBody.get(),
        sqlite3_column_blob(stmt, 2),
        recordBodyBytesCount);

    size_t dataBufferOffset = 0;
    PaymentRecord::SerializedPaymentOperationType *operationType
        = new (recordBody.get() + dataBufferOffset) PaymentRecord::SerializedPaymentOperationType;
    dataBufferOffset += sizeof(
        PaymentRecord::SerializedPaymentOperationType);

    NodeUUID contractorUUID(recordBody.get() + dataBufferOffset);
    dataBufferOffset += NodeUUID::kBytesSize;

    vector<byte> amountBytes(
        recordBody.get() + dataBufferOffset,
        recordBody.get() + dataBufferOffset + kTrustLineAmountBytesCount);
    TrustLineAmount amount = bytesToTrustLineAmount(
        amountBytes);
    dataBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBytes(
        recordBody.get() + dataBufferOffset,
        recordBody.get() + dataBufferOffset + kTrustLineBalanceSerializeBytesCount);
    TrustLineBalance balanceAfterOperation = bytesToTrustLineBalance(
        balanceBytes);

    return make_shared<PaymentRecord>(
        operationUUID,
        (PaymentRecord::PaymentOperationType) *operationType,
        contractorUUID,
        amount,
        balanceAfterOperation,
        timestamp);
}

PaymentRecord::Shared HistoryStorage::deserializePaymentAdditionalRecord(
        sqlite3_stmt *stmt)
{
    TransactionUUID operationUUID((uint8_t *)sqlite3_column_blob(stmt, 0));
    GEOEpochTimestamp timestamp = (GEOEpochTimestamp)sqlite3_column_int64(stmt, 1);
    size_t recordBodyBytesCount = (size_t)sqlite3_column_int(stmt, 3);
    BytesShared recordBody = tryMalloc(recordBodyBytesCount);
    memcpy(
        recordBody.get(),
        sqlite3_column_blob(stmt, 2),
        recordBodyBytesCount);

    size_t dataBufferOffset = 0;
    PaymentRecord::SerializedPaymentOperationType *operationType
        = new (recordBody.get() + dataBufferOffset) PaymentRecord::SerializedPaymentOperationType;
    dataBufferOffset += sizeof(
        PaymentRecord::SerializedPaymentOperationType);

    vector<byte> amountBytes(
        recordBody.get() + dataBufferOffset,
        recordBody.get() + dataBufferOffset + kTrustLineAmountBytesCount);
    TrustLineAmount amount = bytesToTrustLineAmount(
            amountBytes);

    return make_shared<PaymentRecord>(
        operationUUID,
        (PaymentRecord::PaymentOperationType) *operationType,
        amount,
        timestamp);
}

LoggerStream HistoryStorage::info() const
{
    return mLog.info(logHeader());
}

LoggerStream HistoryStorage::error() const
{
    return mLog.error(logHeader());
}

const string HistoryStorage::logHeader() const
{
    stringstream s;
    s << "[HistoryStorage]";
    return s.str();
}

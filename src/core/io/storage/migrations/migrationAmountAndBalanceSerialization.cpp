#include "migrationAmountAndBalanceSerialization.h"

AmountAndBalanceSerializationMigration::AmountAndBalanceSerializationMigration(
        sqlite3 *dbConnection,
        Logger &logger):
        mDataBase(dbConnection),
        mLog(logger)
{}

int AmountAndBalanceSerializationMigration::apply(
        IOTransaction::Shared ioTransaction)
{
    try {
        migrateTrustLines(ioTransaction);
        migratePaymentsHistory(ioTransaction);
        migrateTrustLinesHistory(ioTransaction);
    }
    catch(const std::exception &e){
        mLog.logException("AmountAndBalanceSerializationMigration", e);
        return -1;
    }
    return 0;
}

void AmountAndBalanceSerializationMigration::migrateTrustLines(
        IOTransaction::Shared ioTransaction)
{

    string queryCount = "SELECT count(*) FROM " + ioTransaction->trustLineHandler()->tableName();
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDataBase, queryCount.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::trustLines: "
                              "Bad count query; sqlite error: " + rc);
    }
    sqlite3_step(stmt);
    uint32_t rowCount = (uint32_t)sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    vector<TrustLine::Shared> result;
    result.reserve(rowCount);

    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " +
            ioTransaction->trustLineHandler()->tableName();

    rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::trustLines: "
                              "Bad query; sqlite error: " + rc);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID contractor((uint8_t*)sqlite3_column_blob(stmt, 0));

        byte* incomingAmountBytes = (byte*)sqlite3_column_blob(stmt, 1);
        vector<byte> incomingAmountBufferBytes(
                incomingAmountBytes,
                incomingAmountBytes + kTrustLineAmountBytesCount);
        TrustLineAmount incomingAmount = bytesToTrustLineAmountOld(incomingAmountBufferBytes);

        byte* outgoingAmountBytes = (byte*)sqlite3_column_blob(stmt, 2);
        vector<byte> outgoingAmountBufferBytes(
                outgoingAmountBytes,
                outgoingAmountBytes + kTrustLineAmountBytesCount);
        TrustLineAmount outgoingAmount = bytesToTrustLineAmountOld(outgoingAmountBufferBytes);

        byte* balanceBytes = (byte*)sqlite3_column_blob(stmt, 3);
        vector<byte> balanceBufferBytes(
                balanceBytes,
                balanceBytes + kTrustLineBalanceSerializeBytesCount);
        TrustLineBalance balance = bytesToTrustLineBalanceOld(balanceBufferBytes);
        try {
            result.push_back(
                    make_shared<TrustLine>(
                            contractor,
                            incomingAmount,
                            outgoingAmount,
                            balance));
        } catch (...) {
            throw Exception("AmountAndBalanceSerializationMigration::loadTrustLine. "
                                    "Unable to create trust line instance from DB.");
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    // Repalace old methods with new serialization
    for(auto trustline: result){
        ioTransaction->trustLineHandler()->saveTrustLine(trustline);
    }
}

void AmountAndBalanceSerializationMigration::migratePaymentsHistory(
        IOTransaction::Shared ioTransaction)
{
    auto paymentsRecords = allMainPaymentRecords(ioTransaction);
    for(auto paymentRecord:paymentsRecords){
        updateMainPaymentRecord(
                paymentRecord,
                ioTransaction);
    }
    auto additionalPaymentsRecords = allAdditionalPaymentRecords(ioTransaction);
    for(auto paymentRecord:paymentsRecords){
        updateAdditionalPaymentRecord(
                paymentRecord,
                ioTransaction);
    }
}

void AmountAndBalanceSerializationMigration::migrateTrustLinesHistory(
        IOTransaction::Shared ioTransaction)
{
    auto trustLineRecords = allTrustLinesRecords(ioTransaction);
    for(auto trustLineRecord:trustLineRecords){
        updateTrustLineRecord(
                trustLineRecord,
                ioTransaction);
    }
}

TrustLineAmount AmountAndBalanceSerializationMigration::bytesToTrustLineAmountOld(
        const vector<byte> &amountBytes)
{
    TrustLineAmount amount;

    vector<byte> amountNotZeroBytes;
    amountNotZeroBytes.reserve(kTrustLineAmountBytesCount);

    for (const auto &item : amountBytes) {
        if (item != 0) {
            amountNotZeroBytes.push_back(item);
        }
    }

    if (amountNotZeroBytes.size() > 0) {
        import_bits(
                amount,
                amountNotZeroBytes.begin(),
                amountNotZeroBytes.end()
        );

    } else {
        import_bits(
                amount,
                amountBytes.begin(),
                amountBytes.end()
        );
    }

    return amount;
}

TrustLineBalance AmountAndBalanceSerializationMigration::bytesToTrustLineBalanceOld(
        const vector<byte> balanceBytes)
{

    TrustLineBalance balance;

    vector<byte> notZeroBytesVector;
    notZeroBytesVector.reserve(kTrustLineBalanceBytesCount);

    byte sign = balanceBytes.at(balanceBytes.size() - 1);

    for (size_t byteIndex = 0; byteIndex < balanceBytes.size(); ++ byteIndex) {
        if (byteIndex != balanceBytes.size() - 1) {
            byte byteValue = balanceBytes.at(byteIndex);
            if (byteValue != 0) {
                notZeroBytesVector.push_back(byteValue);
            }
        }
    }

    if (notZeroBytesVector.size() > 0) {
        import_bits(
                balance,
                notZeroBytesVector.begin(),
                notZeroBytesVector.end()
        );

    } else {
        import_bits(
                balance,
                balanceBytes.begin(),
                balanceBytes.end()
        );
    }

    if (sign == 1) {
        balance = balance * -1;
    }

    return balance;
}

vector<PaymentRecord::Shared> AmountAndBalanceSerializationMigration::allMainPaymentRecords(
        IOTransaction::Shared ioTransaction)
{
    vector<PaymentRecord::Shared> result;
    string query = "SELECT operation_uuid, operation_timestamp, record_body, record_body_bytes_count FROM "
                   + ioTransaction->historyStorage()->mainTableName() + " WHERE record_type = ? ";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::allPaymentRecords: "
                              "Bad query; sqlite error: " + rc);
    }
    int idxParam = 1;
    rc = sqlite3_bind_int(stmt, idxParam++, Record::RecordType::PaymentRecordType);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::allPaymentRecords: "
                              "Bad binding of RecordType; sqlite error: " + rc);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result.push_back(
                deserializePaymentRecordOld(
                        stmt));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<PaymentRecord::Shared> AmountAndBalanceSerializationMigration::allAdditionalPaymentRecords(
        IOTransaction::Shared ioTransaction)
{
    vector<PaymentRecord::Shared> result;
    string query = "SELECT operation_uuid, operation_timestamp, record_body, record_body_bytes_count FROM "
                   + ioTransaction->historyStorage()->additionalTableName() + ";";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::allPaymentAdditionalRecords: "
                              "Bad query; sqlite error: " + rc);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result.push_back(
                deserializePaymentAdditionalRecordOld(
                        stmt));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

vector<TrustLineRecord::Shared> AmountAndBalanceSerializationMigration::allTrustLinesRecords(
        IOTransaction::Shared ioTransaction)
{
    vector<TrustLineRecord::Shared> result;
    string query = "SELECT operation_uuid, operation_timestamp, record_body, record_body_bytes_count FROM "
                   + ioTransaction->historyStorage()->mainTableName() + " WHERE record_type = ? ";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::allPaymentRecords: "
                              "Bad query; sqlite error: " + rc);
    }
    int idxParam = 1;
    rc = sqlite3_bind_int(stmt, idxParam++, Record::RecordType::TrustLineRecordType);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::allTrustLinesRecords: "
                              "Bad binding of RecordType; sqlite error: " + rc);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result.push_back(
                deserializeTrustLineRecordOld(
                        stmt));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}

PaymentRecord::Shared AmountAndBalanceSerializationMigration::deserializePaymentAdditionalRecordOld(
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
    TrustLineAmount amount = bytesToTrustLineAmountOld(
            amountBytes);

    return make_shared<PaymentRecord>(
            operationUUID,
            (PaymentRecord::PaymentOperationType) *operationType,
            amount,
            timestamp);
}

PaymentRecord::Shared AmountAndBalanceSerializationMigration::deserializePaymentRecordOld(
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
    TrustLineAmount amount = bytesToTrustLineAmountOld(
            amountBytes);
    dataBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBytes(
            recordBody.get() + dataBufferOffset,
            recordBody.get() + dataBufferOffset + kTrustLineBalanceSerializeBytesCount);
    TrustLineBalance balanceAfterOperation = bytesToTrustLineBalanceOld(
            balanceBytes);

    return make_shared<PaymentRecord>(
            operationUUID,
            (PaymentRecord::PaymentOperationType) *operationType,
            contractorUUID,
            amount,
            balanceAfterOperation,
            timestamp);
}

TrustLineRecord::Shared AmountAndBalanceSerializationMigration::deserializeTrustLineRecordOld(
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

        amount = bytesToTrustLineAmountOld(
                amountBytes);
    }
    return make_shared<TrustLineRecord>(
            operationUUID,
            (TrustLineRecord::TrustLineOperationType) *operationType,
            contractorUUID,
            amount,
            timestamp);
}

void AmountAndBalanceSerializationMigration::updateTrustLineRecord(
        TrustLineRecord::Shared trustLineRecord,
        IOTransaction::Shared ioTransaction)
{
    string query = "UPDATE " + ioTransaction->historyStorage()->mainTableName()
                   + " SET record_body = ? "
                       "WHERE operation_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::update trustline payment: "
                              "Bad query; sqlite error: " + rc);
    }

    auto serializedTrustlineRecordAndSize = serializedTrustLineRecordBody(
            trustLineRecord);

    rc = sqlite3_bind_blob(stmt, 1, serializedTrustlineRecordAndSize.first.get(),
                           (int) serializedTrustlineRecordAndSize.second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::update trustline payment: "
                              "Bad binding of RecordBody; sqlite error: " + rc);
    }

    rc = sqlite3_bind_blob(stmt, 2, trustLineRecord->operationUUID().data, Record::kOperationUUIDBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::update trustline payment: "
                              "Bad binding of OperationUUID; sqlite error: " + rc);
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting of main payment is completed successfully";
#endif
    } else {
        throw IOError("AmountAndBalanceSerializationMigration::insert main payment: "
                              "Run query; sqlite error: " + rc);
    }
}

void AmountAndBalanceSerializationMigration::updateAdditionalPaymentRecord(
        PaymentRecord::Shared paymentRecord,
        IOTransaction::Shared ioTransaction)
{
    string query = "UPDATE " + ioTransaction->historyStorage()->additionalTableName()
                   + " SET record_body = ? "
                           "WHERE operation_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::insert additional payment: "
                              "Bad query; sqlite error: " + rc);
    }

    auto serializedPymentRecordAndSize = serializedAdditionalPaymentRecordBody(
            paymentRecord);

    rc = sqlite3_bind_blob(stmt, 1, serializedPymentRecordAndSize.first.get(),
                           (int) serializedPymentRecordAndSize.second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::insert additional payment: "
                              "Bad binding of RecordBody; sqlite error: " + rc);
    }

    rc = sqlite3_bind_blob(stmt, 2, paymentRecord->operationUUID().data, Record::kOperationUUIDBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::insert additional payment: "
                              "Bad binding of OperationUUID; sqlite error: " + rc);
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting of main payment is completed successfully";
#endif
    } else {
        throw IOError("AmountAndBalanceSerializationMigration::insert main payment: "
                              "Run query; sqlite error: " + rc);
    }
}

void AmountAndBalanceSerializationMigration::updateMainPaymentRecord(
        PaymentRecord::Shared paymentRecord,
        IOTransaction::Shared ioTransaction)
{
    string query = "UPDATE " + ioTransaction->historyStorage()->mainTableName()
                   + " SET record_body = ? "
                           "WHERE operation_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::insert main payment: "
                              "Bad query; sqlite error: " + rc);
    }

    auto serializedPymentRecordAndSize = serializedMainPaymentRecordBody(
            paymentRecord);

    rc = sqlite3_bind_blob(stmt, 1, serializedPymentRecordAndSize.first.get(),
                           (int) serializedPymentRecordAndSize.second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::insert main payment: "
                              "Bad binding of RecordBody; sqlite error: " + rc);
    }

    rc = sqlite3_bind_blob(stmt, 2, paymentRecord->operationUUID().data, Record::kOperationUUIDBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("AmountAndBalanceSerializationMigration::insert main payment: "
                              "Bad binding of OperationUUID; sqlite error: " + rc);
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting of main payment is completed successfully";
#endif
    } else {
        throw IOError("AmountAndBalanceSerializationMigration::insert main payment: "
                              "Run query; sqlite error: " + rc);
    }
}

pair<BytesShared, size_t> AmountAndBalanceSerializationMigration::serializedMainPaymentRecordBody(
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

pair<BytesShared, size_t> AmountAndBalanceSerializationMigration::serializedAdditionalPaymentRecordBody(
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

pair<BytesShared, size_t> AmountAndBalanceSerializationMigration::serializedTrustLineRecordBody(
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
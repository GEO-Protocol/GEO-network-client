#include "PositiveSignMigration.h"

PositiveSignMigration::PositiveSignMigration(
    sqlite3 *dbConnection,
    Logger &logger):

    mDataBase(dbConnection),
    mLog(logger)
{}

void PositiveSignMigration::apply(
    IOTransaction::Shared ioTransaction)
{
    migrateTrustLines(ioTransaction);
    migratePaymentsHistory(ioTransaction);
}

void PositiveSignMigration::migrateTrustLines(
    IOTransaction::Shared ioTransaction)
{
    sqlite3_stmt *stmt;
    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " +
                   ioTransaction->trustLineHandler()->tableName();

    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "PositiveSignMigration::migrateTrustLines: "
                "Can't select trust lines count; SQLite error code: " + to_string(rc));
    }
    vector<TrustLine::Shared> result;
    // For each trust line in the table, deserialize it with old method and serialize it back with new one.
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID contractor(static_cast<const byte *>(sqlite3_column_blob(stmt, 0)));

        auto incomingAmountBytes = static_cast<const byte*>(sqlite3_column_blob(stmt, 1));
        vector<byte> incomingAmountBufferBytes(
            incomingAmountBytes,
            incomingAmountBytes + kTrustLineAmountBytesCount);
        TrustLineAmount incomingAmount = bytesToTrustLineAmount(incomingAmountBufferBytes);

        auto outgoingAmountBytes = static_cast<const byte*>(sqlite3_column_blob(stmt, 2));
        vector<byte> outgoingAmountBufferBytes(
            outgoingAmountBytes,
            outgoingAmountBytes + kTrustLineAmountBytesCount);
        TrustLineAmount outgoingAmount = bytesToTrustLineAmount(outgoingAmountBufferBytes);

        auto balanceBytes = static_cast<const byte*>(sqlite3_column_blob(stmt, 3));
        vector<byte> balanceBufferBytes(
            balanceBytes,
            balanceBytes + kTrustLineBalanceSerializeBytesCount);
        TrustLineBalance balance = bytesToTrustLineBalanceOld(balanceBufferBytes);

        try {
            auto tl = make_shared<TrustLine>(
                contractor,
                incomingAmount,
                outgoingAmount,
                balance);
            result.push_back(tl);
        } catch (...) {
            throw RuntimeError(
                "PositiveSignMigration::migrateTrustLines: "
                    "Unable to migrate TL.");
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    for(auto trustline: result){
        ioTransaction->trustLineHandler()->saveTrustLine(trustline);
    }
}

void PositiveSignMigration::migratePaymentsHistory(
    IOTransaction::Shared ioTransaction)
{
    auto paymentsRecords = allMainPaymentRecords(ioTransaction);
    for(auto paymentRecord: paymentsRecords){
        updateMainPaymentRecord(
            paymentRecord,
            ioTransaction);
    }
}

TrustLineBalance PositiveSignMigration::bytesToTrustLineBalanceOld(
    const vector<byte> balanceBytes)
{
    vector<byte> internalBytesBuffer;
    internalBytesBuffer.reserve(kTrustLineAmountBytesCount);

    // Note: sign byte must be skipped, so the cycle is starting from 1.
    for (size_t i=1; i<kTrustLineAmountBytesCount+1; ++i) {
        internalBytesBuffer.push_back(
            boost::endian::big_to_native(
                balanceBytes[i]));
    }

    TrustLineBalance balance;
    import_bits(
        balance,
        internalBytesBuffer.begin(),
        internalBytesBuffer.end());

    // Sign must be processed only in case if balance != 0.
    // By default, after deserialization, balance is always positive,
    // so it must be only checked for > 0, and not != 0.
    if (balance > 0) {
        byte sign = boost::endian::big_to_native(balanceBytes[0]);
        if (sign == 0) {
            balance = balance * -1;
        }
    }

    return balance;
}

vector<PaymentRecord::Shared> PositiveSignMigration::allMainPaymentRecords(
    IOTransaction::Shared ioTransaction)
{
    vector<PaymentRecord::Shared> records;
    string query = "SELECT operation_uuid, operation_timestamp, record_body, record_body_bytes_count FROM "
                   + ioTransaction->historyStorage()->mainTableName() + " WHERE record_type = ? ";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "PositiveSignMigration::allPaymentRecords: "
                "Bad query; sqlite error: " + to_string(rc));
    }

    int idxParam = 1;
    rc = sqlite3_bind_int(stmt, idxParam, Record::RecordType::PaymentRecordType);
    if (rc != SQLITE_OK) {
        throw IOError(
            "PositiveSignMigration::allPaymentRecords: "
                "Bad binding of RecordType; sqlite error: " + to_string(rc));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        records.push_back(
            deserializePaymentRecordOld(
                stmt));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return records;
}

PaymentRecord::Shared PositiveSignMigration::deserializePaymentRecordOld(
    sqlite3_stmt *stmt)
{
    TransactionUUID operationUUID(static_cast<const byte*>(sqlite3_column_blob(stmt, 0)));
    GEOEpochTimestamp timestamp = static_cast<GEOEpochTimestamp>(sqlite3_column_int64(stmt, 1));
    size_t recordBodyBytesCount = static_cast<const size_t>(sqlite3_column_int(stmt, 3));
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
    TrustLineBalance balanceAfterOperation = bytesToTrustLineBalanceOld(
        balanceBytes);

    return make_shared<PaymentRecord>(
        operationUUID,
        static_cast<PaymentRecord::PaymentOperationType>(*operationType),
        contractorUUID,
        amount,
        balanceAfterOperation,
        timestamp);
}

void PositiveSignMigration::updateMainPaymentRecord(
    PaymentRecord::Shared paymentRecord,
    IOTransaction::Shared ioTransaction)
{
    string query = "UPDATE " + ioTransaction->historyStorage()->mainTableName()
                   + " SET record_body = ? "
                       "WHERE operation_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PositiveSignMigration::insert main payment: "
                          "Bad query; sqlite error: "+ to_string(rc));
    }

    auto serializedPymentRecordAndSize = serializedMainPaymentRecordBody(
        paymentRecord);

    rc = sqlite3_bind_blob(stmt, 1, serializedPymentRecordAndSize.first.get(),
                           (int) serializedPymentRecordAndSize.second, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PositiveSignMigration::insert main payment: "
                          "Bad binding of RecordBody; sqlite error: "+ to_string(rc));
    }

    rc = sqlite3_bind_blob(stmt, 2, paymentRecord->operationUUID().data, Record::kOperationUUIDBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PositiveSignMigration::insert main payment: "
                          "Bad binding of OperationUUID; sqlite error: "+ to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting of main payment is completed successfully";
#endif
    } else {
        throw IOError("PositiveSignMigration::insert main payment: "
                          "Run query; sqlite error: "+ to_string(rc));
    }
}

pair<BytesShared, size_t> PositiveSignMigration::serializedMainPaymentRecordBody(
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

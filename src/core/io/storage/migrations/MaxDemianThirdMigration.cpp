#include "MaxDemianThirdMigration.h"

MaxDemianThirdMigration::MaxDemianThirdMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mDataBase(dbConnection),
    mLog(logger)
{
    mNeighborUUID = NodeUUID("0bc6a4fa-154a-4cbd-9409-79d0a1c3e2f1");

    mOperationAmount = TrustLineAmount(36456);

    mOperationTimeStampOnCoordinatorNode = 46599671500257;
}

void MaxDemianThirdMigration::applyTrustLineMigration(
    IOTransaction::Shared ioTransaction)
{
    auto trustLine = getOutwornTrustLine(ioTransaction, mNeighborUUID);

    if (trustLine) {

        auto new_balance = trustLine->balance() - mOperationAmount;
        trustLine->setBalance(new_balance);
        ioTransaction->trustLinesHandler()->saveTrustLine(trustLine);

    } else {
        throw RuntimeError("Can not find TrustLines to migrate");
    }
}

void MaxDemianThirdMigration::applyTrustLineHistoryMigration(IOTransaction::Shared ioTransaction)
{

    auto previousPaymentRecord = getPreviousPaymentRecord(ioTransaction);

    auto timestamp = mOperationTimeStampOnCoordinatorNode;

    if (microsecondsSinceGEOEpoch(previousPaymentRecord->timestamp()) > mOperationTimeStampOnCoordinatorNode){
        timestamp = microsecondsSinceGEOEpoch(previousPaymentRecord->timestamp()) + 1;
    }

    auto paymentRecordsForUpdate = getPaymentRecordsForUpdate(ioTransaction, timestamp);

    updatePaymentsRecords(ioTransaction, paymentRecordsForUpdate);
}

void MaxDemianThirdMigration::apply(IOTransaction::Shared ioTransaction) {

    applyTrustLineMigration(ioTransaction);
    applyTrustLineHistoryMigration(ioTransaction);
}

std::shared_ptr <TrustLine> MaxDemianThirdMigration::getOutwornTrustLine(
    IOTransaction::Shared ioTransaction,
    NodeUUID &nodeUUID) {
    sqlite3_stmt *stmt;
    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " +
                   ioTransaction->trustLinesHandler()->tableName() + " WHERE contractor = ?";


    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "MaxDemianThirdMigration::getOutwornTrustLine: "
                "Can't select trust lines count; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_bind_blob(stmt, 1, nodeUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError(
            "TrustLineHandler::insert or replace: "
                "Bad binding of Contractor; sqlite error: " + to_string(rc));
    }

    if (sqlite3_step(stmt) == SQLITE_ROW ) {
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
        TrustLineBalance balance = bytesToTrustLineBalance(balanceBufferBytes);

        try {

            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);

            return make_shared<TrustLine>(
                contractor,
                incomingAmount,
                outgoingAmount,
                balance,
                false);

        } catch (...) {
            throw RuntimeError(
                "MaxDemianThirdMigration::getOutwornTrustLine: "
                    "Unable to create TL.");
        }
    } else {

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);

        throw NotFoundError(
            "MaxDemianThirdMigration::getOutwornTrustLine: "
                "Unable to find TL.");
    }
}



PaymentRecord::Shared MaxDemianThirdMigration::getPreviousPaymentRecord(
    IOTransaction::Shared ioTransaction)
{
    sqlite3_stmt *stmt;

    string query = "SELECT operation_uuid,"
                       " operation_timestamp,"
                       " record_body,"
                       " record_body_bytes_count from "
                   + ioTransaction->historyStorage()->mainTableName() + " where operation_timestamp < ? "
                       "ORDER BY operation_timestamp DESC LIMIT 1";

    PaymentRecord::Shared result;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "MaxDemianThirdMigration::getPreviousPaymentRecord: "
                "Can't select PaymentRecord; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_bind_int64(stmt, 1, mOperationTimeStampOnCoordinatorNode);
    if (rc != SQLITE_OK) {
        throw IOError(
            "MaxDemianThirdMigration::getPreviousPaymentRecord:  "
                "Can't select PaymentRecord; SQLite error code:: " + to_string(rc));
    }
    if (sqlite3_step(stmt) == SQLITE_ROW ) {
        result = deserializePaymentRecord(stmt);
    } else {
        throw NotFoundError("MaxDemianThirdMigration::getPreviousPaymentRecord:  "
                                "Cannot Find Previous record");
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}


PaymentRecord::Shared MaxDemianThirdMigration::deserializePaymentRecord(
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

void MaxDemianThirdMigration::updatePaymentsRecords(
    IOTransaction::Shared ioTransaction,
    vector<PaymentRecord::Shared> paymentsRecordsToUpdate)
{
    for(auto record: paymentsRecordsToUpdate){
        stringstream ss;
        sqlite3_stmt *stmt;
        string query = " UPDATE " + ioTransaction->historyStorage()->mainTableName() +
                       " SET record_body = ?" +
                       " WHERE operation_uuid = ?;";

        int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            throw IOError(
                "MaxDemianThirdMigration::updatePaymentsRecords: "
                    "Can't update PaymentRecord; SQLite error code: " + to_string(rc));
        }

        auto serializedPaymentRecordAndSize = serializedPaymentRecordBody(
            record);
        rc = sqlite3_bind_blob(stmt, 1, serializedPaymentRecordAndSize.first.get(),
                               (int) serializedPaymentRecordAndSize.second, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            throw IOError("TrustLineHandler::updatePaymentsRecords: "
                              "Bad binding of record_body; sqlite error: " + to_string(rc));
        }

        rc = sqlite3_bind_blob(stmt, 2, record->operationUUID().data, NodeUUID::kBytesSize, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            throw IOError(
                "MaxDemianThirdMigration::updatePaymentsRecords "
                    "Bad binding of Operation_uuid; sqlite error: " + to_string(rc));
        }
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE){
            throw IOError("HistoryStorage::insert trustline: "
                              "Run query; sqlite error: " + to_string(rc));
        }
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE){
            throw IOError("HistoryStorage::insert trustline: "
                              "Run query; sqlite error: " + to_string(rc));
        }
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    }
}

vector<PaymentRecord::Shared> MaxDemianThirdMigration::getPaymentRecordsForUpdate(
    IOTransaction::Shared ioTransaction,
    int64_t since_what_timestamp)
{
    sqlite3_stmt *stmt;
    string query = "SELECT operation_uuid,"
                       " operation_timestamp,"
                       " record_body,"
                       " record_body_bytes_count FROM "
                   + ioTransaction->historyStorage()->mainTableName() + " where record_type = ?;";

    vector<PaymentRecord::Shared> recordsToUpdate;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "SolomonHistoryMigration::getPreviousPaymentRecord: "
                "Can't select PaymentRecord; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_bind_int(stmt, 1, Record::RecordType::PaymentRecordType);
    if (rc != SQLITE_OK) {
        throw IOError("SolomonHistoryMigration::insert or replace: "
                          "Bad binding of Contractor; sqlite error: " + rc);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        TransactionUUID operationUUID((uint8_t *)sqlite3_column_blob(stmt, 0));
        GEOEpochTimestamp timestamp = (GEOEpochTimestamp)sqlite3_column_int64(stmt, 1);

        // Filter records update in C++ Code
        if (timestamp <= since_what_timestamp)
            continue;

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
        // Update Balance
        balanceAfterOperation = balanceAfterOperation - mOperationAmount;

        recordsToUpdate.push_back(make_shared<PaymentRecord>(
            operationUUID,
            (PaymentRecord::PaymentOperationType) *operationType,
            contractorUUID,
            amount,
            balanceAfterOperation,
            timestamp));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return recordsToUpdate;
}

pair<BytesShared, size_t> MaxDemianThirdMigration::serializedPaymentRecordBody(
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
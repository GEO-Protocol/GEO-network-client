#include "SolomonHistoryMigration.h"

SolomonHistoryMigration::SolomonHistoryMigration(
    sqlite3 *dbConnection,
    Logger &logger):
    mLog(logger),
    mDataBase(dbConnection)
{
    mMigrateNodeUUID = NodeUUID("2136a78d-3cb0-488d-b1a6-e039c12689d0");
    mCoordinatorUUID = NodeUUID("ba684d43-a66e-4840-9b29-679dcd679f23");
    mNeighborUUID = NodeUUID("15ce1178-ba9d-4172-845d-48d005dae106");
    mOperationTimeStampOnCoordinatorNode = 44791775256137;

    mOperationUUID = TransactionUUID(string("a4bebc20-8c87-448c-9a92-c4f86e28d2e2"));
    mPreviousOperationUUID = TransactionUUID(string("11809a84-8301-4c9f-84c1-bc7820968c47"));

    mOperationAmount = TrustLineAmount(2282);
}

void SolomonHistoryMigration::apply(
    IOTransaction::Shared ioTransaction)
{

    applyTrustLineMigration(ioTransaction);
    applyTrustLineHistoryMigration(ioTransaction);
}

void SolomonHistoryMigration::applyTrustLineMigration(
    IOTransaction::Shared ioTransaction)
{
    auto trustLine = getOutwornTrustLine(ioTransaction);
    auto new_balance = trustLine->balance() + mOperationAmount;
    trustLine->setBalance(new_balance);
    ioTransaction->trustLineHandler()->saveTrustLine(trustLine);
}

void SolomonHistoryMigration::applyTrustLineHistoryMigration(IOTransaction::Shared ioTransaction)
{

    auto previousPaymentRecord = getPreviousPaymentRecord(ioTransaction);

    auto balanceAfterOperation = previousPaymentRecord->balanceAfterOperation() + mOperationAmount;

    auto timestamp = mOperationTimeStampOnCoordinatorNode;
    if (microsecondsSinceGEOEpoch(previousPaymentRecord->timestamp()) > mOperationTimeStampOnCoordinatorNode){
        timestamp = microsecondsSinceGEOEpoch(previousPaymentRecord->timestamp()) + 1;
    }

    auto missedPaymentRecord = make_shared<PaymentRecord>(
        mOperationUUID,
        PaymentRecord::PaymentOperationType::IncomingPaymentType,
        mCoordinatorUUID,
        mOperationAmount,
        balanceAfterOperation,
        timestamp);

//    ioTransaction->historyStorage()-> savePaymentRecord(missedPaymentRecord);
    cout << timestamp << endl;
    auto paymentRecordsForUpdate = getPaymentRecordsForUpdate(ioTransaction, timestamp);

    updatePaymentsRecords(ioTransaction, paymentRecordsForUpdate);
}


vector<PaymentRecord::Shared> SolomonHistoryMigration::getPaymentRecordsForUpdate(
    IOTransaction::Shared ioTransaction,
    int64_t since_what_timestamp)
{
    sqlite3_stmt *stmt;
    string query = "SELECT operation_uuid,"
                       " operation_timestamp,"
                       " record_body,"
                       " record_body_bytes_count from "
                   + ioTransaction->historyStorage()->mainTableName() + " where (operation_timestamp > ?) and (record_type = ?);";

    vector<PaymentRecord::Shared> recordsToUpdate;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "SolomonHistoryMigration::getPreviousPaymentRecord: "
                "Can't select PaymentRecord; SQLite error code: " + to_string(rc));
    }
    // todo ask about fit in int
    cout << "TimeStamp : " << since_what_timestamp << endl;
    rc = sqlite3_bind_int(stmt, 1, since_what_timestamp);
    if (rc != SQLITE_OK) {
        throw IOError("SolomonHistoryMigration::insert or replace: "
                          "Bad binding of Contractor; sqlite error: " + rc);
    }
    rc = sqlite3_bind_int(stmt, 2, Record::RecordType::PaymentRecordType);
    if (rc != SQLITE_OK) {
        throw IOError("SolomonHistoryMigration::insert or replace: "
                          "Bad binding of Contractor; sqlite error: " + rc);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW ) {
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
        // Update Balance
        balanceAfterOperation = balanceAfterOperation + mOperationAmount;

        recordsToUpdate.push_back(make_shared<PaymentRecord>(
            operationUUID,
            (PaymentRecord::PaymentOperationType) *operationType,
            contractorUUID,
            amount,
            balanceAfterOperation,
            timestamp));
    }
    cout << recordsToUpdate.size() << endl;
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return recordsToUpdate;
}

TrustLine::Shared SolomonHistoryMigration::getOutwornTrustLine(
    IOTransaction::Shared ioTransaction)
{
    sqlite3_stmt *stmt;
    string query = "SELECT contractor, incoming_amount, outgoing_amount, balance FROM " +
                   ioTransaction->trustLineHandler()->tableName() + " where contractor = ?";


    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "SolomonHistoryMigration::getOutwornTrustLine: "
                "Can't select trust lines count; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_bind_blob(stmt, 1, mNeighborUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("TrustLineHandler::insert or replace: "
                          "Bad binding of Contractor; sqlite error: " + rc);
    }
    TrustLine::Shared result;
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
        TrustLineBalance balance = bytesToTrustLineBalance(balanceBufferBytes);

        try {
            return make_shared<TrustLine>(
                contractor,
                incomingAmount,
                outgoingAmount,
                balance);

        } catch (...) {
            throw RuntimeError(
                "SolomonHistoryMigration::getOutwornTrustLine: "
                    "Unable to create TL.");
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}


PaymentRecord::Shared SolomonHistoryMigration::getPreviousPaymentRecord(
    IOTransaction::Shared ioTransaction)
{
    sqlite3_stmt *stmt;

    string query = "SELECT operation_uuid,"
        " operation_timestamp,"
        " record_body,"
        " record_body_bytes_count from "
                   + ioTransaction->historyStorage()->mainTableName() + " where operation_uuid = ?;";

    PaymentRecord::Shared result;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError(
            "SolomonHistoryMigration::getPreviousPaymentRecord: "
                "Can't select PaymentRecord; SQLite error code: " + to_string(rc));
    }

    rc = sqlite3_bind_blob(stmt, 1, mPreviousOperationUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("SolomonHistoryMigration::insert or replace: "
                          "Bad binding of Contractor; sqlite error: " + rc);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        result = deserializePaymentRecord(stmt);
        break;
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return result;
}


PaymentRecord::Shared SolomonHistoryMigration::deserializePaymentRecord(
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

void SolomonHistoryMigration::updatePaymentsRecords(
    IOTransaction::Shared ioTransaction,
    vector<PaymentRecord::Shared> paymentsRecordsToUpdate)
{
    cout << "Payments records to update" << endl;
    cout << paymentsRecordsToUpdate.size() << endl;
    for(auto record: paymentsRecordsToUpdate){
        stringstream ss;
        sqlite3_stmt *stmt;
        string query = " UPDATE " + ioTransaction->historyStorage()->mainTableName() +
                       " SET record_body = ?" +
                        " WHERE operation_uuid = ?;";

        int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            throw IOError(
                "SolomonHistoryMigration::updatePaymentsRecords: "
                    "Can't update PaymentRecord; SQLite error code: " + to_string(rc));
        }

        auto serializedPaymentRecordAndSize = serializedPaymentRecordBody(
            record);
        ss << record->operationUUID() << "||" << record->balanceAfterOperation();
        cout << ss.str() <<endl;
        rc = sqlite3_bind_blob(stmt, 1, serializedPaymentRecordAndSize.first.get(),
                               (int) serializedPaymentRecordAndSize.second, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            throw IOError("TrustLineHandler::updatePaymentsRecords: "
                              "Bad binding of record_body; sqlite error: " + rc);
        }

        rc = sqlite3_bind_blob(stmt, 2, record->operationUUID().data, NodeUUID::kBytesSize, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            throw IOError(
                "SolomonHistoryMigration::updatePaymentsRecords "
                    "Bad binding of Operation_uuid; sqlite error: " + rc);
        }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    }
}

pair<BytesShared, size_t> SolomonHistoryMigration::serializedPaymentRecordBody(
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
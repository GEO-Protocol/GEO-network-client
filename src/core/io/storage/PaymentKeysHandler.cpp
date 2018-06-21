#include "PaymentKeysHandler.h"

PaymentKeysHandler::PaymentKeysHandler(
    sqlite3 *dbConnection,
    const string &tableName,
    Logger &logger) :

    mDataBase(dbConnection),
    mTableName(tableName),
    mLog(logger)
{
    sqlite3_stmt *stmt;
    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                   " (transaction_uuid BLOB NOT NULL, "
                   "participant_uuid BLOB NOT NULL, "
                   "public_key BLOB NOT NULL, "
                   "private_key BLOB NOT NULL);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::creating table: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PaymentKeysHandler::creating table: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_transaction_uuid_idx on " + mTableName + "(transaction_uuid);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::creating  index for TransactionUUID: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PaymentKeysHandler::creating index for TransactionUUID: "
                          "Run query; sqlite error: " + to_string(rc));
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void PaymentKeysHandler::saveOwnKey(
    const TransactionUUID &transactionUUID,
    const NodeUUID &ownNodeUUID,
    const PublicKey::Shared publicKey,
    const PrivateKey *privateKey)
{
    string query = "INSERT INTO " + mTableName +
                   "(transaction_uuid, participant_uuid, public_key, private_key) "
                   "VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::saveOwnKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data,
                           TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::saveOwnKey: "
                          "Bad binding of Transaction UUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 2, ownNodeUUID.data,
                           NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::saveOwnKey: "
                          "Bad binding of Own Node UUID; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 3, publicKey->data(),
                           (int)publicKey->keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::saveOwnKey: "
                          "Bad binding of Public Key; sqlite error: " + to_string(rc));
    }

    // todo encrypt private key data
    BytesShared buffer = tryMalloc(privateKey->keySize());
    {
        auto g = privateKey->data()->unlockAndInitGuard();
        memcpy(
            buffer.get(),
            g.address(),
            privateKey->keySize());
    }
    auto g = privateKey->data()->unlockAndInitGuard();
    rc = sqlite3_bind_blob(stmt, 4, buffer.get(),
                       (int) privateKey->keySize(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::saveOwnKey: "
                          "Bad binding of Private Key; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting is completed successfully";
#endif
    } else {
        throw IOError("PaymentKeysHandler::saveOwnKey: "
                          "Run query; sqlite error: " + to_string(rc));
    }
}

PrivateKey* PaymentKeysHandler::getOwnPrivateKey(
    const TransactionUUID &transactionUUID)
{
    string query = "SELECT private_key FROM " + mTableName
                   + " WHERE transaction_uuid = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::getOwnPrivateKey: "
                          "Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_bind_blob(stmt, 1, transactionUUID.data, TransactionUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("PaymentKeysHandler::getOwnPrivateKey: "
                          "Bad binding of Transaction UUID; sqlite error: " + to_string(rc));
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        auto result = new PrivateKey((byte*)sqlite3_column_blob(stmt, 0));
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    } else {
        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        throw NotFoundError("PaymentKeysHandler::getOwnPrivateKey: "
                                "There are now records with requested transaction UUID");
    }
}

LoggerStream PaymentKeysHandler::info() const
{
    return mLog.info(logHeader());
}

LoggerStream PaymentKeysHandler::warning() const
{
    return mLog.warning(logHeader());
}

const string PaymentKeysHandler::logHeader() const
{
    stringstream s;
    s << "[PaymentKeysHandler]";
    return s.str();
}
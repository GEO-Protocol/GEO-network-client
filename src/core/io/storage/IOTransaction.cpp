#include "IOTransaction.h"

IOTransaction::IOTransaction(
    sqlite3 *dbConnection,
    TrustLineHandler *trustLineHandler,
    HistoryStorage *historyStorage,
    TransactionsHandler *transactionHandler,
    BlackListHandler *blackListHandler,
    OwnKeysHandler *ownKeysHandler,
    ContractorKeysHandler *contractorKeysHandler,
    AuditHandler *auditHandler,
    IncomingPaymentReceiptHandler *incomingPaymentReceiptHandler,
    OutgoingPaymentReceiptHandler *outgoingPaymentReceiptHandler,
    PaymentKeysHandler *paymentKeysHandler,
    PaymentParticipantsVotesHandler *paymentParticipantsVotesHandler,
    Logger &logger) :

    mDBConnection(dbConnection),
    mTrustLineHandler(trustLineHandler),
    mHistoryStorage(historyStorage),
    mTransactionHandler(transactionHandler),
    mBlackListHandler(blackListHandler),
    mOwnKeysHandler(ownKeysHandler),
    mContractorKeysHandler(contractorKeysHandler),
    mAuditHandler(auditHandler),
    mIncomingPaymentReceiptHandler(incomingPaymentReceiptHandler),
    mOutgoingPaymentReceiptHandler(outgoingPaymentReceiptHandler),
    mPaymentKeysHandler(paymentKeysHandler),
    mPaymentParticipantsVotesHandler(paymentParticipantsVotesHandler),
    mIsTransactionBegin(true),
    mLog(logger)
{
    beginTransactionQuery();
}

IOTransaction::~IOTransaction()
{
    commit();
}

TrustLineHandler* IOTransaction::trustLinesHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::trustLineHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mTrustLineHandler;
}

HistoryStorage* IOTransaction::historyStorage()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::historyStorage: "
                          "transaction was rollback, it can't be use now");
    }
    return mHistoryStorage;
}

TransactionsHandler* IOTransaction::transactionHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::transactionHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mTransactionHandler;
}

OwnKeysHandler* IOTransaction::ownKeysHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::ownKeysHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mOwnKeysHandler;
}

ContractorKeysHandler* IOTransaction::contractorKeysHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::contractorKeysHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mContractorKeysHandler;
}

AuditHandler* IOTransaction::auditHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::auditHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mAuditHandler;
}

IncomingPaymentReceiptHandler* IOTransaction::incomingPaymentReceiptHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::incomingPaymentReceiptHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mIncomingPaymentReceiptHandler;
}

OutgoingPaymentReceiptHandler* IOTransaction::outgoingPaymentReceiptHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::outgoingPaymentReceiptHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mOutgoingPaymentReceiptHandler;
}

PaymentKeysHandler* IOTransaction::paymentKeysHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::PaymentKeysHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mPaymentKeysHandler;
}

PaymentParticipantsVotesHandler* IOTransaction::paymentParticipantsVotesHandler()
{
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::paymentParticipantsVotesHandler: "
                          "transaction was rollback, it can't be use now");
    }
    return mPaymentParticipantsVotesHandler;
}

void IOTransaction::commit()
{
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "commit";
#endif
    if (!mIsTransactionBegin) {
        warning() << "transaction don't commit, because it wasn't started";
        return;
    }
    string query = "COMMIT TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IOTransaction::commit: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("IOTransaction::commit: Run query; sqlite error: " + to_string(rc));
    }
    mIsTransactionBegin = false;
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction commit";
#endif
}

void IOTransaction::rollback()
{
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "rollback";
#endif
    string query = "ROLLBACK TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IOTransaction::rollback: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("IOTransaction::rollback: Run query; sqlite error: " + to_string(rc));
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "rollBack done";
#endif
    mIsTransactionBegin = false;
}

LoggerStream IOTransaction::info() const
{
    return mLog.info(logHeader());
}

LoggerStream IOTransaction::warning() const
{
    return mLog.warning(logHeader());
}

const string IOTransaction::logHeader() const
{
    stringstream s;
    s << "[IOTransaction]";
    return s.str();
}

void IOTransaction::beginTransactionQuery() {
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "beginTransactionQuery";
#endif
    string query = "BEGIN TRANSACTION;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(mDBConnection, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("IOTransaction::prepareInserted: Bad query; sqlite error: " + to_string(rc));
    }
    rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw IOError("IOTransaction::prepareInserted: Run query; sqlite error: " + to_string(rc));
    }
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "transaction begin";
#endif
}

BlackListHandler *IOTransaction::blackListHandler() {
    if (!mIsTransactionBegin) {
        throw IOError("IOTransaction::historyStorage: "
                          "transaction was rollback, it can't be use now");
    }
    return mBlackListHandler;
}

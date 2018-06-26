#ifndef GEO_NETWORK_CLIENT_IOTRANSACTION_H
#define GEO_NETWORK_CLIENT_IOTRANSACTION_H

#include "../../common/Types.h"
#include "TrustLineHandler.h"
#include "HistoryStorage.h"
#include "TransactionsHandler.h"
#include "BlackListHandler.h"

#include "OwnKeysHandler.h"
#include "ContractorKeysHandler.h"
#include "AuditHandler.h"
#include "IncomingPaymentReceiptHandler.h"
#include "OutgoingPaymentReceiptHandler.h"
#include "PaymentKeysHandler.h"
#include "PaymentParticipantsVotesHandler.h"

#include "../../../libs/sqlite3/sqlite3.h"

class IOTransaction {

public:
    typedef shared_ptr<IOTransaction> Shared;

public:
    IOTransaction(
        sqlite3 *dbConnection,
        TrustLineHandler *trustLinesHandler,
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
        Logger &logger);

    ~IOTransaction();

    TrustLineHandler *trustLinesHandler();

    HistoryStorage *historyStorage();

    TransactionsHandler *transactionHandler();

    BlackListHandler *blackListHandler();

    OwnKeysHandler *ownKeysHandler();

    ContractorKeysHandler *contractorKeysHandler();

    AuditHandler *auditHandler();

    IncomingPaymentReceiptHandler *incomingPaymentReceiptHandler();

    OutgoingPaymentReceiptHandler *outgoingPaymentReceiptHandler();

    PaymentKeysHandler *paymentKeysHandler();

    PaymentParticipantsVotesHandler *paymentParticipantsVotesHandler();

    void rollback();

private:

    void commit();

    void beginTransactionQuery();

    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDBConnection;
    TrustLineHandler *mTrustLineHandler;
    HistoryStorage *mHistoryStorage;
    TransactionsHandler *mTransactionHandler;
    BlackListHandler *mBlackListHandler;

    OwnKeysHandler *mOwnKeysHandler;
    ContractorKeysHandler *mContractorKeysHandler;
    AuditHandler *mAuditHandler;
    IncomingPaymentReceiptHandler *mIncomingPaymentReceiptHandler;
    OutgoingPaymentReceiptHandler *mOutgoingPaymentReceiptHandler;
    PaymentKeysHandler *mPaymentKeysHandler;
    PaymentParticipantsVotesHandler *mPaymentParticipantsVotesHandler;

    bool mIsTransactionBegin;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_IOTRANSACTION_H

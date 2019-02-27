#ifndef GEO_NETWORK_CLIENT_IOTRANSACTION_H
#define GEO_NETWORK_CLIENT_IOTRANSACTION_H

#include "../../common/Types.h"
#include "TrustLineHandler.h"
#include "HistoryStorage.h"
#include "TransactionsHandler.h"

#include "OwnKeysHandler.h"
#include "ContractorKeysHandler.h"
#include "AuditHandler.h"
#include "IncomingPaymentReceiptHandler.h"
#include "OutgoingPaymentReceiptHandler.h"
#include "PaymentKeysHandler.h"
#include "PaymentParticipantsVotesHandler.h"
#include "PaymentTransactionsHandler.h"

#include "ContractorsHandler.h"
#include "AddressHandler.h"

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
        OwnKeysHandler *ownKeysHandler,
        ContractorKeysHandler *contractorKeysHandler,
        AuditHandler *auditHandler,
        IncomingPaymentReceiptHandler *incomingPaymentReceiptHandler,
        OutgoingPaymentReceiptHandler *outgoingPaymentReceiptHandler,
        PaymentKeysHandler *paymentKeysHandler,
        PaymentParticipantsVotesHandler *paymentParticipantsVotesHandler,
        PaymentTransactionsHandler *paymentTransactionsHandler,
        ContractorsHandler *contractorsHandler,
        AddressHandler *addressHandler,
        Logger &logger);

    ~IOTransaction();

    TrustLineHandler *trustLinesHandler();

    HistoryStorage *historyStorage();

    TransactionsHandler *transactionHandler();

    OwnKeysHandler *ownKeysHandler();

    ContractorKeysHandler *contractorKeysHandler();

    AuditHandler *auditHandler();

    IncomingPaymentReceiptHandler *incomingPaymentReceiptHandler();

    OutgoingPaymentReceiptHandler *outgoingPaymentReceiptHandler();

    PaymentKeysHandler *paymentKeysHandler();

    PaymentParticipantsVotesHandler *paymentParticipantsVotesHandler();

    PaymentTransactionsHandler *paymentTransactionsHandler();

    ContractorsHandler *contractorsHandler();

    AddressHandler *addressHandler();

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

    OwnKeysHandler *mOwnKeysHandler;
    ContractorKeysHandler *mContractorKeysHandler;
    AuditHandler *mAuditHandler;
    IncomingPaymentReceiptHandler *mIncomingPaymentReceiptHandler;
    OutgoingPaymentReceiptHandler *mOutgoingPaymentReceiptHandler;
    PaymentKeysHandler *mPaymentKeysHandler;
    PaymentParticipantsVotesHandler *mPaymentParticipantsVotesHandler;
    PaymentTransactionsHandler *mPaymentTransactionsHandler;

    ContractorsHandler *mContractorsHandler;
    AddressHandler *mAddressHandler;

    bool mIsTransactionBegin;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_IOTRANSACTION_H

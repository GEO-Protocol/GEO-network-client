#ifndef GEO_NETWORK_CLIENT_STORAGEHANDLER_H
#define GEO_NETWORK_CLIENT_STORAGEHANDLER_H

#include "../../logger/Logger.h"
#include "TrustLineHandler.h"
#include "TransactionsHandler.h"
#include "HistoryStorage.h"
#include "BlackListHandler.h"
#include "OwnKeysHandler.h"
#include "ContractorKeysHandler.h"
#include "AuditHandler.h"
#include "IncomingPaymentReceiptHandler.h"
#include "OutgoingPaymentReceiptHandler.h"
#include "PaymentKeysHandler.h"
#include "PaymentParticipantsVotesHandler.h"
#include "../../common/exceptions/IOError.h"
#include "../../../libs/sqlite3/sqlite3.h"
#include "IOTransaction.h"

#include <boost/filesystem.hpp>
#include <vector>

namespace fs = boost::filesystem;

class StorageHandler {

public:
    StorageHandler(
        const string &directory,
        const string &dataBaseName,
        Logger &logger);

    ~StorageHandler();

    IOTransaction::Shared beginTransaction();

private:
    static void checkDirectory(
        const string &directory);

    static sqlite3* connection(
        const string &dataBaseName,
        const string &directory);

    LoggerStream info() const;

    LoggerStream warning() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    const string kTrustLineTableName = "trust_lines";
    const string kTransactionTableName = "transactions";
    const string kHistoryMainTableName = "history";
    const string kHistoryAdditionalTableName = "history_additional";
    const string kBlackListTableName = "blacklist";

    const string kOwnKeysTableName = "own_keys";
    const string kContractorKeysTableName = "contractor_keys";
    const string kOutgoingReceiptTableName = "outgoing_receipt";
    const string kIncomingReceiptTableName = "incoming_receipt";
    const string kAuditTableName = "audit";
    const string kPaymentKeysTableName = "payment_keys";
    const string kPaymentParticipantsVotesTableName = "payment_participants_votes";

private:
    static sqlite3 *mDBConnection;

private:
    Logger &mLog;
    TrustLineHandler mTrustLineHandler;
    TransactionsHandler mTransactionHandler;
    HistoryStorage mHistoryStorage;
    BlackListHandler mBlackListHandler;
    OwnKeysHandler mOwnKeysHandler;
    ContractorKeysHandler mContractorKeysHandler;
    AuditHandler mAuditHandler;
    IncomingPaymentReceiptHandler mIncomingPaymentReceiptHandler;
    OutgoingPaymentReceiptHandler mOutgoingPaymentReceiptHandler;
    PaymentKeysHandler mPaymentKeysHandler;
    PaymentParticipantsVotesHandler mPaymentParticipantsVotesHandler;
    string mDirectory;
    string mDataBaseName;
};


#endif //GEO_NETWORK_CLIENT_STORAGEHANDLER_H

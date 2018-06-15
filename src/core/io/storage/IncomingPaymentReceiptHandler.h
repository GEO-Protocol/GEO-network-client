#ifndef GEO_NETWORK_CLIENT_INCOMINGPAYMENTRECEIPTHANDLER_H
#define GEO_NETWORK_CLIENT_INCOMINGPAYMENTRECEIPTHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"
#include "../../crypto/lamportscheme.h"

#include "../../../libs/sqlite3/sqlite3.h"

using namespace crypto::lamport;

class IncomingPaymentReceiptHandler {

public:
    IncomingPaymentReceiptHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveRecord(
        const TrustLineID trustLineID,
        const AuditNumber auditNumber,
        const TransactionUUID &transactionUUID,
        const uint32_t contractorPublicKeyHash,
        const TrustLineAmount &amount,
        const Signature::Shared contractorSignature);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_INCOMINGPAYMENTRECEIPTHANDLER_H

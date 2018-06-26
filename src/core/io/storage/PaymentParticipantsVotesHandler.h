#ifndef GEO_NETWORK_CLIENT_PAYMENTPARTICIPANTSVOTESHANDLER_H
#define GEO_NETWORK_CLIENT_PAYMENTPARTICIPANTSVOTESHANDLER_H

#include "../../logger/Logger.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/Types.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/NotFoundError.h"

#include "../../../libs/sqlite3/sqlite3.h"
#include "../../crypto/lamportkeys.h"
#include "../../crypto/lamportscheme.h"

using namespace crypto;

class PaymentParticipantsVotesHandler {

public:
    PaymentParticipantsVotesHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveRecord(
        const TransactionUUID &transactionUUID,
        const NodeUUID &nodeUUID,
        const PaymentNodeID paymentNodeID,
        const lamport::PublicKey::Shared publicKey,
        const lamport::Signature::Shared signature);

    map<PaymentNodeID, lamport::Signature::Shared> participantsSignatures(
        const TransactionUUID &transactionUUID);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_PAYMENTPARTICIPANTSVOTESHANDLER_H

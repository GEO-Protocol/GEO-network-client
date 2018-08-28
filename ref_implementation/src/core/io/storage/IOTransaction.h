/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_IOTRANSACTION_H
#define GEO_NETWORK_CLIENT_IOTRANSACTION_H

#include "../../common/Types.h"
#include "TrustLineHandler.h"
#include "HistoryStorage.h"
#include "PaymentOperationStateHandler.h"
#include "TransactionsHandler.h"
#include "BlackListHandler.h"
#include "NodeFeaturesHandler.h"

#include "../../../libs/sqlite3/sqlite3.h"

class IOTransaction {

public:
    typedef shared_ptr<IOTransaction> Shared;

public:
    IOTransaction(
        sqlite3 *dbConnection,
        TrustLineHandler *trustLinesHandler,
        HistoryStorage *historyStorage,
        PaymentOperationStateHandler *paymentOperationStorage,
        TransactionsHandler *transactionHandler,
        BlackListHandler *blackListHandler,
        NodeFeaturesHandler *nodeFeaturesHandler,
        Logger &logger);

    ~IOTransaction();

    TrustLineHandler *trustLinesHandler();

    HistoryStorage *historyStorage();

    PaymentOperationStateHandler *paymentOperationStateHandler();

    TransactionsHandler *transactionHandler();

    BlackListHandler *blackListHandler();

    NodeFeaturesHandler *nodeFeaturesHandler();

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
    PaymentOperationStateHandler *mPaymentOperationStateHandler;
    TransactionsHandler *mTransactionHandler;
    BlackListHandler *mBlackListHandler;
    NodeFeaturesHandler *mNodeFeaturesHandler;
    bool mIsTransactionBegin;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_IOTRANSACTION_H

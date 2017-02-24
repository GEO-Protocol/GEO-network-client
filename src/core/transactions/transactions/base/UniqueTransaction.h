#ifndef GEO_NETWORK_CLIENT_UNIQUETRANSACTION_H
#define GEO_NETWORK_CLIENT_UNIQUETRANSACTION_H

#include "BaseTransaction.h"

#include "TransactionUUID.h"
#include "../../../common/NodeUUID.h"

#include "../../scheduler/TransactionsScheduler.h"

class UniqueTransaction : public BaseTransaction{

protected:
    UniqueTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        TransactionsScheduler *scheduler);

    UniqueTransaction(
        const TransactionType type,
        TransactionsScheduler *scheduler);

    void killTransaction(
        const TransactionUUID &transactionUUID);

    const map<BaseTransaction::Shared, TransactionState::SharedConst>* pendingTransactions();

private:
    TransactionsScheduler *mTransactionsScheduler;
};


#endif //GEO_NETWORK_CLIENT_UNIQUETRANSACTION_H

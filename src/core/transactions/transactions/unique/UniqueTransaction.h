#ifndef GEO_NETWORK_CLIENT_UNIQUETRANSACTION_H
#define GEO_NETWORK_CLIENT_UNIQUETRANSACTION_H

#include "../BaseTransaction.h"

#include "../../scheduler/TransactionsScheduler.h"

class UniqueTransaction : public BaseTransaction{

protected:
    UniqueTransaction(
        TransactionType type,
        NodeUUID &nodeUUID,
        TransactionsScheduler *scheduler);

    const map<BaseTransaction::Shared, TransactionState::SharedConst>* pendingTransactions();

private:
    TransactionsScheduler *mTransactionsScheduler;
};


#endif //GEO_NETWORK_CLIENT_UNIQUETRANSACTION_H

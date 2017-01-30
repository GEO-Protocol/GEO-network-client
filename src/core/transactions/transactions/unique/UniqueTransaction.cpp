#include "UniqueTransaction.h"

UniqueTransaction::UniqueTransaction(
    BaseTransaction::TransactionType type,
    NodeUUID &nodeUUID,
    TransactionsScheduler *scheduler) :

    BaseTransaction(
        type,
        nodeUUID
    ),
    mTransactionsScheduler(scheduler) {}

UniqueTransaction::UniqueTransaction(
    TransactionsScheduler *scheduler) :

    mTransactionsScheduler(scheduler) {}


const map<BaseTransaction::Shared, TransactionState::SharedConst>* UniqueTransaction::pendingTransactions() {

    return transactions(mTransactionsScheduler);
}


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


void UniqueTransaction::killTransaction(
    const TransactionUUID &transactionUUID) {

    mTransactionsScheduler->killTransaction(transactionUUID);
}

const map<BaseTransaction::Shared, TransactionState::SharedConst>* UniqueTransaction::pendingTransactions() {

    return transactions(mTransactionsScheduler);
}
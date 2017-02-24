#include "UniqueTransaction.h"

UniqueTransaction::UniqueTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    TransactionsScheduler *scheduler) :

    BaseTransaction(
        type,
        nodeUUID
    ),
    mTransactionsScheduler(scheduler) {}

UniqueTransaction::UniqueTransaction(
    const BaseTransaction::TransactionType type,
    TransactionsScheduler *scheduler) :

    BaseTransaction(
        type
    ),
    mTransactionsScheduler(scheduler) {}


void UniqueTransaction::killTransaction(
    const TransactionUUID &transactionUUID) {

    mTransactionsScheduler->killTransaction(transactionUUID);
}

const map<BaseTransaction::Shared, TransactionState::SharedConst>* UniqueTransaction::pendingTransactions() {

    return transactions(mTransactionsScheduler);
}
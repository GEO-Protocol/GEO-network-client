#include "UniqueTransaction.h"

UniqueTransaction::UniqueTransaction(
    BaseTransaction::TransactionType type,
    TransactionsScheduler *scheduler) :

    BaseTransaction(type),
    mTransactionsScheduler(scheduler) {}

const map<Shared, TransactionState::SharedConst>* UniqueTransaction::pendingTransactions() {

    return transactions(mTransactionsScheduler);
}


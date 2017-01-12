#include "TransactionsObserver.h"

TransactionsObserver::TransactionsObserver(
    TransactionsScheduler *scheduler) :

    mScheduler(scheduler){}

TransactionsObserver::~TransactionsObserver() {
}

const bool TransactionsObserver::isSameTypeTransactionExist(
    BaseTransaction::Shared transaction) const {

    //return mScheduler->isTransactionInScheduler(transaction);
}
